#include "cutiesysteminfo.h"

#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QStorageInfo>

// ============================================================
// CutieOSInfo Implementation
// ============================================================

CutieOSInfo::CutieOSInfo(QObject *parent)
    : QObject(parent) {
    m_osName = getOSName();
    m_kernel = executeCommand("uname -r");
    m_build = readFile("/etc/os-release");
    m_channel = readFile("/etc/lsb-release-codename").trimmed();
}

QString CutieOSInfo::getOSName() {
    QString osRelease = readFile("/etc/os-release");
    
    for (const QString &line : osRelease.split('\n')) {
        if (line.startsWith("NAME=")) {
            return line.mid(6).remove('"');
        }
    }
    
    QString lsbRelease = readFile("/etc/lsb-release");
    for (const QString &line : lsbRelease.split('\n')) {
        if (line.startsWith("DISTRIB_DESCRIPTION=")) {
            return line.mid(20).remove('"');
        }
    }
    
    return "Linux";
}

QString CutieOSInfo::readFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "Unknown";
    }

    QString content = file.readAll();
    file.close();
    return content.trimmed();
}

QString CutieOSInfo::executeCommand(const QString &command) {
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << command);

    if (!process.waitForFinished()) {
        return "Unknown";
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

// ============================================================
// CutieHardwareInfo Implementation
// ============================================================

CutieHardwareInfo::CutieHardwareInfo(QObject *parent)
    : QObject(parent) {
    m_device = getDeviceName();
    m_processor = getProcessorName();
    m_memory = getTotalMemory();
    m_storage = getStorageInfo();
    m_display = getDisplayInfo();
    m_battery = getBatteryInfo();
}

QString CutieHardwareInfo::getDeviceName() {
    QString deviceName = executeCommand("cat /sys/firmware/devicetree/base/model 2>/dev/null");
    
    if (deviceName.isEmpty() || deviceName == "Unknown") {
        deviceName = executeCommand("cat /sys/class/dmi/id/product_name 2>/dev/null");
    }
    
    if (deviceName.isEmpty()) {
        deviceName = "Generic Device";
    }
    
    return deviceName;
}

QString CutieHardwareInfo::getProcessorName() {
    QString cpuInfo = readFile("/proc/cpuinfo");
    
    for (const QString &line : cpuInfo.split('\n')) {
        if (line.startsWith("model name")) {
            QString modelName = line.split(':')[1].trimmed();
            
            modelName = modelName.replace("(R)", "").replace("(TM)", "")
                                 .replace("CPU", "").replace("@", "").trimmed();
            
            if (modelName.contains("GHz")) {
                modelName = modelName.left(modelName.indexOf("GHz")).trimmed();
            }
            
            return modelName;
        }
    }
    
    return "Unknown";
}

QString CutieHardwareInfo::getTotalMemory() {
    QString memInfo = readFile("/proc/meminfo");
    
    for (const QString &line : memInfo.split('\n')) {
        if (line.startsWith("MemTotal:")) {
            QString kbStr = line.split(':')[1].trimmed().split(' ')[0];
            bool ok;
            long long kb = kbStr.toLongLong(&ok);
            
            if (ok) {
                double gb = kb / (1024.0 * 1024.0);
                return QString::number(gb, 'f', 1) + " GB";
            }
        }
    }
    
    return "Unknown";
}

QString CutieHardwareInfo::getStorageInfo() {
    QStorageInfo storage(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    
    if (storage.isValid()) {
        double totalGB = storage.bytesTotal() / (1024.0 * 1024.0 * 1024.0);
        return QString::number(totalGB, 'f', 0) + " GB";
    }
    
    return "Unknown";
}

QString CutieHardwareInfo::getDisplayInfo() {
    QString displayInfo = executeCommand("xdpyinfo -display $DISPLAY 2>/dev/null | grep dimensions | awk '{print $2}'");
    
    if (displayInfo.isEmpty() || displayInfo == "Unknown") {
        displayInfo = executeCommand("wlr-randr 2>/dev/null | grep -i connected | head -1 | awk '{print $3}'");
    }
    
    if (displayInfo.isEmpty()) {
        displayInfo = "Unknown";
    }
    
    return displayInfo;
}

QString CutieHardwareInfo::getBatteryInfo() {
    QString batteryPath = "/sys/class/power_supply/BAT0/capacity";
    QString batteryStatus = readFile(batteryPath);
    
    if (batteryStatus != "Unknown") {
        bool ok;
        int percentage = batteryStatus.toInt(&ok);
        if (ok) {
            return QString::number(percentage) + "%";
        }
    }
    
    batteryPath = "/sys/class/power_supply/BAT1/capacity";
    batteryStatus = readFile(batteryPath);
    
    if (batteryStatus != "Unknown") {
        bool ok;
        int percentage = batteryStatus.toInt(&ok);
        if (ok) {
            return QString::number(percentage) + "%";
        }
    }
    
    return "N/A";
}

QString CutieHardwareInfo::readFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "Unknown";
    }

    QString content = file.readAll();
    file.close();
    return content.trimmed();
}

QString CutieHardwareInfo::executeCommand(const QString &command) {
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << command);

    if (!process.waitForFinished()) {
        return "Unknown";
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

// ============================================================
// CutieSystemInfo Implementation
// ============================================================

CutieSystemInfo::CutieSystemInfo(QObject *parent)
    : QObject(parent) {
    m_osInfo = new CutieOSInfo(this);
    m_hwInfo = new CutieHardwareInfo(this);
}

CutieSystemInfo::~CutieSystemInfo() {
}