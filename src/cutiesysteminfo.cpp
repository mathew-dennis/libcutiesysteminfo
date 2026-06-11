#include "cutiesysteminfo.h"

#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QStandardPaths>
#include <QStorageInfo>

// ============================================================
// OSInfo Implementation
// ============================================================

OSInfo::OSInfo(QObject *parent)
    : QObject(parent) {
    m_osName = getOSName();
    m_kernel = executeCommand("uname -r");
    m_build = readFile("/etc/os-release");
    m_channel = readFile("/etc/lsb-release-codename").trimmed();
}

QString OSInfo::getOSName() {
    // Try to read from /etc/os-release
    QString osRelease = readFile("/etc/os-release");
    
    // Parse NAME field
    for (const QString &line : osRelease.split('\n')) {
        if (line.startsWith("NAME=")) {
            return line.mid(6).remove('"');
        }
    }
    
    // Fallback: try /etc/lsb-release
    QString lsbRelease = readFile("/etc/lsb-release");
    for (const QString &line : lsbRelease.split('\n')) {
        if (line.startsWith("DISTRIB_DESCRIPTION=")) {
            return line.mid(20).remove('"');
        }
    }
    
    return "Linux";
}

QString OSInfo::readFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "Unknown";
    }

    QString content = file.readAll();
    file.close();
    return content.trimmed();
}

QString OSInfo::executeCommand(const QString &command) {
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << command);

    if (!process.waitForFinished()) {
        return "Unknown";
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

// ============================================================
// HardwareInfo Implementation
// ============================================================

HardwareInfo::HardwareInfo(QObject *parent)
    : QObject(parent) {
    m_device = getDeviceName();
    m_processor = getProcessorName();
    m_memory = getTotalMemory();
    m_storage = getStorageInfo();
    m_display = getDisplayInfo();
    m_battery = getBatteryInfo();
}

QString HardwareInfo::getDeviceName() {
    // Try to get device model from device tree
    QString deviceName = executeCommand("cat /sys/firmware/devicetree/base/model 2>/dev/null");
    
    if (deviceName.isEmpty() || deviceName == "Unknown") {
        // Fallback: try DMI data
        deviceName = executeCommand("cat /sys/class/dmi/id/product_name 2>/dev/null");
    }
    
    if (deviceName.isEmpty()) {
        deviceName = "Generic Device";
    }
    
    return deviceName;
}

QString HardwareInfo::getProcessorName() {
    // Try to get CPU model name from cpuinfo
    QString cpuInfo = readFile("/proc/cpuinfo");
    
    for (const QString &line : cpuInfo.split('\n')) {
        if (line.startsWith("model name")) {
            QString modelName = line.split(':')[1].trimmed();
            
            // Clean up the model name
            modelName = modelName.replace("(R)", "").replace("(TM)", "")
                                 .replace("CPU", "").replace("@", "").trimmed();
            
            // Remove frequency info if present
            if (modelName.contains("GHz")) {
                modelName = modelName.left(modelName.indexOf("GHz")).trimmed();
            }
            
            return modelName;
        }
    }
    
    return "Unknown";
}

QString HardwareInfo::getTotalMemory() {
    // Read from /proc/meminfo
    QString memInfo = readFile("/proc/meminfo");
    
    for (const QString &line : memInfo.split('\n')) {
        if (line.startsWith("MemTotal:")) {
            // Extract KB value and convert to GB
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

QString HardwareInfo::getStorageInfo() {
    // Get storage info for home directory
    QStorageInfo storage(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    
    if (storage.isValid()) {
        double totalGB = storage.bytesTotal() / (1024.0 * 1024.0 * 1024.0);
        return QString::number(totalGB, 'f', 0) + " GB";
    }
    
    return "Unknown";
}

QString HardwareInfo::getDisplayInfo() {
    // Try to get display resolution from X11
    QString displayInfo = executeCommand("xdpyinfo -display $DISPLAY 2>/dev/null | grep dimensions | awk '{print $2}'");
    
    if (displayInfo.isEmpty() || displayInfo == "Unknown") {
        // Fallback: try wayland with wlr-randr
        displayInfo = executeCommand("wlr-randr 2>/dev/null | grep -i connected | head -1 | awk '{print $3}'");
    }
    
    if (displayInfo.isEmpty()) {
        displayInfo = "Unknown";
    }
    
    return displayInfo;
}

QString HardwareInfo::getBatteryInfo() {
    // Try to read from power supply
    QString batteryPath = "/sys/class/power_supply/BAT0/capacity";
    QString batteryStatus = readFile(batteryPath);
    
    if (batteryStatus != "Unknown") {
        bool ok;
        int percentage = batteryStatus.toInt(&ok);
        if (ok) {
            return QString::number(percentage) + "%";
        }
    }
    
    // Try alternative path
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

QString HardwareInfo::readFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "Unknown";
    }

    QString content = file.readAll();
    file.close();
    return content.trimmed();
}

QString HardwareInfo::executeCommand(const QString &command) {
    QProcess process;
    process.start("/bin/sh", QStringList() << "-c" << command);

    if (!process.waitForFinished()) {
        return "Unknown";
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

// ============================================================
// SystemInfo Implementation
// ============================================================

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent) {
    m_osInfo = new OSInfo(this);
    m_hwInfo = new HardwareInfo(this);
}

SystemInfo::~SystemInfo() {
}