#ifndef SAMPLEDATA_H
#define SAMPLEDATA_H

#include <QVariantMap>
#include <QString>
#include <QMetaType>

// This struct can store any number of key/value pairs (the dynamic IDL fields)
// plus two extra fields: timestamp and sampleInfo.
struct SampleData {
    QVariantMap fields;  // dynamic key/value pairs for IDL fields
    QString timestamp;   // e.g., "2025-02-21 16:30:00 1740135600 IST"
    QString sampleInfo;  // extra sample information (could be multiline)
};
 
Q_DECLARE_METATYPE(SampleData)

#endif // SAMPLEDATA_H
