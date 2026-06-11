#pragma once

#include <QtQuick>
#include <QString>

// ============================================================
// OS Information Object (Internal)
// ============================================================
class CutieOSInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(QString build READ build CONSTANT)
    Q_PROPERTY(QString kernel READ kernel CONSTANT)
    Q_PROPERTY(QString channel READ channel CONSTANT)

public:
    explicit CutieOSInfo(QObject *parent = nullptr);

    QString osName() const { return m_osName; }
    QString build() const { return m_build; }
    QString kernel() const { return m_kernel; }
    QString channel() const { return m_channel; }

private:
    QString m_osName;
    QString m_build;
    QString m_kernel;
    QString m_channel;

    QString readFile(const QString &filePath);
    QString executeCommand(const QString &command);
    QString getOSName();
};

// ============================================================
// Hardware Information Object (Internal)
// ============================================================
class CutieHardwareInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString device READ device CONSTANT)
    Q_PROPERTY(QString processor READ processor CONSTANT)
    Q_PROPERTY(QString memory READ memory CONSTANT)
    Q_PROPERTY(QString storage READ storage CONSTANT)
    Q_PROPERTY(QString display READ display CONSTANT)
    Q_PROPERTY(QString battery READ battery CONSTANT)

public:
    explicit CutieHardwareInfo(QObject *parent = nullptr);

    QString device() const { return m_device; }
    QString processor() const { return m_processor; }
    QString memory() const { return m_memory; }
    QString storage() const { return m_storage; }
    QString display() const { return m_display; }
    QString battery() const { return m_battery; }

private:
    QString m_device;
    QString m_processor;
    QString m_memory;
    QString m_storage;
    QString m_display;
    QString m_battery;

    QString readFile(const QString &filePath);
    QString executeCommand(const QString &command);
    
    QString getDeviceName();
    QString getProcessorName();
    QString getTotalMemory();
    QString getStorageInfo();
    QString getDisplayInfo();
    QString getBatteryInfo();
};

// ============================================================
// Main System Info Provider (Exposed to QML)
// ============================================================
class CutieSystemInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(CutieOSInfo *osInfo READ getOSInfo CONSTANT)
    Q_PROPERTY(CutieHardwareInfo *hwInfo READ getHWInfo CONSTANT)

public:
    explicit CutieSystemInfo(QObject *parent = nullptr);
    ~CutieSystemInfo();

    CutieOSInfo *getOSInfo() const { return m_osInfo; }
    CutieHardwareInfo *getHWInfo() const { return m_hwInfo; }

private:
    CutieOSInfo *m_osInfo;
    CutieHardwareInfo *m_hwInfo;
};