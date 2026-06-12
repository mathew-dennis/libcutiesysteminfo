#include "cutiesysteminfo.h"

#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QStorageInfo>

// Helper function to extract individual properties from key-value system files cleanly
QString getOsReleaseValue(const QString &filePath, const QString &key) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }

    QString content = file.readAll();
    file.close();

    for (const QString &line : content.split('\n')) {
        if (line.startsWith(key + "=")) {
            QString val = line.mid(key.length() + 1).trimmed();
            if (val.startsWith('"') && val.endsWith('"')) {
                val = val.mid(1, val.length() - 2);
            }
            return val;
        }
    }
    return "";
}

// ============================================================
// CutieOSInfo Implementation
// ============================================================

CutieOSInfo::CutieOSInfo(QObject *parent)
    : QObject(parent) {
    
    // Fix: Targets PRETTY_NAME for a beautiful OS label output
    m_osName = getOsReleaseValue("/etc/os-release", "PRETTY_NAME");
    if (m_osName.isEmpty()) {
        m_osName = getOsReleaseValue("/etc/os-release", "NAME");
    }
    if (m_osName.isEmpty()) {
        m_osName = "Linux";
    }

    m_kernel = executeCommand("uname -r");

    // Fix: Stopped reading whole file, extracts target version/build numbers gracefully
    m_build = getOsReleaseValue("/etc/os-release", "VERSION_ID");
    if (m_build.isEmpty()) {
        m_build = getOsReleaseValue("/etc/os-release", "BUILD_ID");
    }
    if (m_build.isEmpty()) {
        m_build = "1.0";
    }

    m_channel = readFile("/etc/lsb-release-codename").trimmed();
    if (m_channel == "Unknown" || m_channel.isEmpty()) {
        m_channel = getOsReleaseValue("/etc/os-release", "VERSION_CODENAME");
    }
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
    
    // Remove null bytes introduced by direct sysfs kernel file reading
    deviceName.remove(QChar('\0')); 
    
    if (deviceName.isEmpty() || deviceName == "Unknown") {
        deviceName = executeCommand("cat /sys/class/dmi/id/product_name 2>/dev/null");
        deviceName.remove(QChar('\0'));
    }
    
    if (deviceName.isEmpty()) {
        deviceName = "Generic Device";
    }
    
    // Fix: Strips ugly trailing text variants like (board-id...) and bracket artifacts
    if (deviceName.contains("(")) {
        deviceName = deviceName.left(deviceName.indexOf("(")).trimmed();
    }
    deviceName.remove('[').remove(']');
    
    return deviceName.trimmed();
}

QString CutieHardwareInfo::getProcessorName() {
    QString cpuInfo = readFile("/proc/cpuinfo");
    
    for (const QString &line : cpuInfo.split('\n')) {
        if (line.startsWith("model name") || line.startsWith("Processor")) {
            QString modelName = line.split(':')[1].trimmed();
            
            modelName = modelName.replace("(R)", "").replace("(TM)", "")
                                 .replace("CPU", "").replace("@", "").trimmed();
            
            if (modelName.contains("GHz")) {
                modelName = modelName.left(modelName.indexOf("GHz")).trimmed();
            }
            
            return modelName;
        }
    }
    
    // Fallback path for specific mobile ARM platforms missing model names in cpuinfo
    QString hardwareName = getOsReleaseValue("/proc/cpuinfo", "Hardware");
    if (!hardwareName.isEmpty()) return hardwareName;
    
    return "ARM Processor";
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
    
    // Look for generic mobile power supply nodes if standard laptop batteries aren't found
    QString batteryCapacity = readFile("/sys/class/power_supply/battery/capacity");
    if (batteryCapacity != "Unknown") {
        return batteryCapacity + "%";
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
