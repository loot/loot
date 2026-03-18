#include "detection.h"
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace GUI {
namespace State {
namespace Game {

Detection::Detection(QObject* parent)
    : QObject(parent)
{
}

bool Detection::isValidPlugin(const QString& filename) const
{
    static const QStringList validPlugins = {
        "starfield.esm",
        "blueprintships-constellation.esm",
        "blueprintships-sfta03.esm",
        "blueprintships-sfta06.esm",
        "oldmars.esm",
        "shatteredspace.esm"
    };
    
    QString lowerFilename = filename.toLower();
    return validPlugins.contains(lowerFilename) || 
           lowerFilename.endsWith(".esp") || 
           lowerFilename.endsWith(".esm");
}

bool Detection::isBlueprintShipsPlugin(const QString& filename) const
{
    QString lowerFilename = filename.toLower();
    return lowerFilename.startsWith("blueprintships-");
}

bool Detection::isTrackersAlliancePlugin(const QString& filename) const
{
    QString lowerFilename = filename.toLower();
    return lowerFilename == "blueprintships-sfta03.esm" || 
           lowerFilename == "blueprintships-sfta06.esm";
}

QStringList Detection::detectPlugins(const QString& dataPath) const
{
    QStringList detectedPlugins;
    QDir dataDir(dataPath);
    
    if (!dataDir.exists()) {
        qWarning() << "Data directory does not exist:" << dataPath;
        return detectedPlugins;
    }
    
    QStringList filters;
    filters << "*.esm" << "*.esp";
    
    QFileInfoList files = dataDir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        QString filename = fileInfo.fileName();
        if (isValidPlugin(filename)) {
            detectedPlugins.append(filename);
        }
    }
    
    return detectedPlugins;
}

QString Detection::getPluginType(const QString& filename) const
{
    QString lowerFilename = filename.toLower();
    
    if (lowerFilename == "starfield.esm") {
        return "Base Game";
    } else if (isTrackersAlliancePlugin(filename)) {
        return "Trackers Alliance DLC";
    } else if (isBlueprintShipsPlugin(filename)) {
        return "Blueprint Ships";
    } else if (lowerFilename == "oldmars.esm") {
        return "Old Mars DLC";
    } else if (lowerFilename == "shatteredspace.esm") {
        return "Shattered Space DLC";
    } else if (lowerFilename.endsWith(".esm")) {
        return "Master File";
    } else if (lowerFilename.endsWith(".esp")) {
        return "Plugin";
    }
    
    return "Unknown";
}

bool Detection::hasRequiredPlugins(const QStringList& plugins) const
{
    // Check for base game
    bool hasStarfield = false;
    for (const QString& plugin : plugins) {
        if (plugin.toLower() == "starfield.esm") {
            hasStarfield = true;
            break;
        }
    }
    
    return hasStarfield;
}

} // namespace Game
} // namespace State
} // namespace GUI