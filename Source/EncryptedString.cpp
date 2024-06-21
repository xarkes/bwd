#include "EncryptedString.h"
#include "ThirdParty/aes.h"
#include "BWNetworkService.h"

#include <QList>
#include <QDebug>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>

static bool CompareHash(QByteArray a, QByteArray b)
{
  if (a.length() != b.length()) {
    return false;
  }
  uint8_t r = 0;
  for (qsizetype i = 0; i < a.length(); i++) {
    r += a[i] ^ b[i];
  }
  return r == 0;
}

EncryptedString::EncryptedString(QString str)
{
  if (!str.length()) {
    return;
  }

  QList<QString> splits = str.split(".");
  QList<QString> pieces;

  if (splits.length() == 2) {
    bool ok;
    m_type = (EncryptionType)splits[0].toInt(&ok);
    if (!ok) {
      qCritical() << "Invalid encryption cipher received: " << str;
      return;
    }
    pieces = splits[1].split("|");
  } else {
    pieces = str.split("|");
    m_type = pieces.length() == 3 ? EncryptionType::AesCbc128_HmacSha256_B64 : EncryptionType::AesCbc256_B64;
  }

  switch (this->m_type) {
  case EncryptionType::AesCbc256_B64:
    m_iv = QByteArray::fromBase64(pieces[0].toLatin1());
    m_data = QByteArray::fromBase64(pieces[1].toLatin1());
    break;
  case EncryptionType::AesCbc128_HmacSha256_B64:
  case EncryptionType::AesCbc256_HmacSha256_B64:
    m_iv = QByteArray::fromBase64(pieces[0].toLatin1());
    m_data = QByteArray::fromBase64(pieces[1].toLatin1());
    m_mac = QByteArray::fromBase64(pieces[2].toLatin1());
    break;
  default:
    qCritical() << "Unsupported cipher for EncryptedString initialization: " << m_type;
    return;
  }
}

void EncryptedString::setClear(QString str)
{
  m_decrypted = str.toUtf8();

  if (m_type == EncryptionType::Unknown) {
    // This is the first time this string is being encrypted.
    // Arbitrarily choose the encryption type
    m_type = EncryptionType::AesCbc256_HmacSha256_B64;
  }

  // In any case, generate a new IV
  auto gen = QRandomGenerator();
  m_iv.resize(16);
  gen.generate(m_iv.begin(), m_iv.end());

  switch (m_type) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
  {
    auto key = Net()->m_key.sliced(0, 32);
    auto macKey = Net()->m_key.sliced(32, 32);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (uint8_t*) key.data(), (uint8_t*) m_iv.data());
    m_data = m_decrypted;
    // PKCS7 padding
    int remain = 16 - m_data.length() % 16;
    for (int i = 0; i < remain; i++) {
      m_data.append(remain);
    }
    Q_ASSERT(m_data.length() % 16 == 0);
    AES_CBC_encrypt_buffer(&ctx, (uint8_t*) m_data.data(), m_data.length());

    QByteArray macData = m_iv + m_data;
    m_mac = QMessageAuthenticationCode::hash(macData, macKey, QCryptographicHash::Sha256);
    break;
  }
  default:
    qCritical() << "Unsupported cipher for encryption: " << m_type;
    return;
  }
}

QString EncryptedString::toString()
{
  if (!m_data.length()) {
    // String is empty, return empty string
    return "";
  }

  switch (m_type) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
    return QString("%1.%2|%3|%4").arg(m_type).arg(m_iv.toBase64()).arg(m_data.toBase64()).arg(m_mac.toBase64());
  default:
    qCritical() << "Unsupported cipher for formatting: " << m_type;
    return "";
  }
}

QString EncryptedString::decrypt()
{
  if (m_decrypted.length()) {
    return m_decrypted;
  }
  if (!m_data.length()) {
    // Nothing to decrypt
    return "";
  }

  // XXX: Probably only for AesCbc256_HmacSha256
  auto key = Net()->m_key.sliced(0, 32);
  auto macKey = Net()->m_key.sliced(32, 32);
  decryptToBytes(key, macKey);

  // Cut off PKCS7 padding for strings
  // XXX: Maybe wrong for non aes-cbc encryption
  auto len = m_decrypted[m_decrypted.length() - 1];
  if (len <= 16) {
    m_decrypted.truncate(m_decrypted.length() - len);
  }

  return m_decrypted;
}

QFuture<QString> EncryptedString::decryptAsync()
{
  return QtConcurrent::run(QThreadPool::globalInstance(), [this](){
    return decrypt();
  });
}

// TODO: Need to understand why the original client uses such structure
// and the lifespan of decrypted information
QByteArray EncryptedString::decryptToBytes(QByteArray key, QByteArray mac)
{
  if (m_decrypted.length()) {
    return m_decrypted;
  }
  if (!m_data.length()) {
    // Nothing to decrypt
    return "";
  }

  switch (m_type) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
  {
    if (m_mac.length()) {
      // XXX: Is it expected that some entries have no mac? If so why use a mac at all?
      QByteArray macData = m_iv + m_data;
      QByteArray computedHash = QMessageAuthenticationCode::hash(macData, mac, QCryptographicHash::Sha256);
      if (!CompareHash(computedHash, m_mac)) {
        // XXX: Should we invalidate the data?
        qDebug() << "hmac verif failed";
        qDebug() << "   " << computedHash.toHex();
        qDebug() << "   " << m_mac.toHex();
        qDebug() << "   " << mac.toHex();
      }
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (uint8_t*) key.data(), (uint8_t*) m_iv.data());
    AES_CBC_decrypt_buffer(&ctx, (uint8_t*) m_data.data(), m_data.length());
    m_decrypted = m_data;
    this->m_data = this->m_iv = this->m_mac = "";
    break;
  }
  default:
    qCritical() << "Unsupported cipher! " << m_type;
  }
  return m_decrypted;
}
