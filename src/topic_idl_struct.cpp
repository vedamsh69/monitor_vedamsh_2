#include <include/indedds-monitor/topic_idl_struct.h>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <iostream>

TopicIDLStruct::TopicIDLStruct(QObject *parent) : QObject(parent) 
{
    std::cout << "[TopicIDLStruct] Constructor called" << std::endl;
}

void TopicIDLStruct::setTextData(const QString &text) 
{
    if (m_textData != text) {
        m_textData = text;
        std::cout << "[TopicIDLStruct] TextData updated, length: " << m_textData.length() << std::endl;
        emit textDataChanged();
        parseIDLToTree();
    }
}

// ========== FILE EXPORT IMPLEMENTATION ==========
bool TopicIDLStruct::saveTextToFile(const QString& filePath, const QString& content)
{
    std::cout << "[TopicIDLStruct] ========== SAVING FILE ==========" << std::endl;
    std::cout << "[TopicIDLStruct] File path: " << filePath.toStdString() << std::endl;
    std::cout << "[TopicIDLStruct] Content length: " << content.length() << " bytes" << std::endl;
    
    // Remove file:// prefix if present
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = cleanPath.mid(7);
    }
    
    QFile file(cleanPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cout << "[TopicIDLStruct] ERROR: Cannot open file for writing: " 
                  << file.errorString().toStdString() << std::endl;
        return false;
    }
    
    QTextStream out(&file);
    out << content;
    file.close();
    
    std::cout << "[TopicIDLStruct] ✓ File saved successfully!" << std::endl;
    return true;
}

QString TopicIDLStruct::generateCSVFromTree()
{
    std::cout << "[TopicIDLStruct] ========== GENERATING CSV ==========" << std::endl;
    
    QString csv = "Name,Type/Ordinal,Extensibility,Optional,Min Sample,Max Sample,Type Code,Type Object,Union Labels\n";
    
    // Function to recursively process nodes
    std::function<void(const QVariantMap&, int)> processNode = [&](const QVariantMap& node, int indent) {
        // Add indentation
        QString indentStr = "";
        for (int i = 0; i < indent; i++) {
            indentStr += "  ";
        }
        
        csv += "\"" + indentStr + node["name"].toString() + "\",";
        csv += "\"" + node["typeOrdinal"].toString() + "\",";
        csv += "\"" + node["extensibility"].toString() + "\",";
        csv += "\"" + node["optional"].toString() + "\",";
        csv += "\"" + node["minSample"].toString() + "\",";
        csv += "\"" + node["maxSample"].toString() + "\",";
        csv += "\"" + node["typeCode"].toString() + "\",";
        csv += "\"" + node["typeObject"].toString() + "\",";
        csv += "\"" + node["unionLabels"].toString() + "\"\n";
        
        // Process children
        QVariantList children = node["children"].toList();
        for (const QVariant& childVariant : children) {
            QVariantMap childNode = childVariant.toMap();
            processNode(childNode, indent + 1);
        }
    };
    
    // Process all root nodes
    for (const QVariant& rootVariant : m_treeData) {
        QVariantMap rootNode = rootVariant.toMap();
        processNode(rootNode, 0);
    }
    
    std::cout << "[TopicIDLStruct] ✓ CSV generated, length: " << csv.length() << " bytes" << std::endl;
    return csv;
}

// ... rest of the existing implementation (parseIDLToTree, etc.)

void TopicIDLStruct::parseIDLToTree()
{
    std::cout << "[TopicIDLStruct] ========== Starting IDL parsing ==========" << std::endl;
    m_treeData.clear();
    
    if (m_textData.isEmpty()) {
        std::cout << "[TopicIDLStruct] Empty IDL text" << std::endl;
        emit treeDataChanged();
        return;
    }
    
    parseStructToTree(m_textData);
    std::cout << "[TopicIDLStruct] ✓ Tree generated with " << m_treeData.size() << " root nodes" << std::endl;
    emit treeDataChanged();
}

QVariantMap TopicIDLStruct::createTreeNode(
    const QString& name, const QString& type,
    const QString& extensibility, const QString& optional,
    int minSize, int maxSize, const QString& typeCode,
    const QString& typeObject, const QString& unionLabels,
    int depth)
{
    QVariantMap node;
    node["name"] = name;
    node["typeOrdinal"] = type;
    node["extensibility"] = extensibility;
    node["optional"] = optional;
    node["minSample"] = QString::number(minSize);
    node["maxSample"] = QString::number(maxSize);
    node["typeCode"] = typeCode;
    node["typeObject"] = typeObject;
    node["unionLabels"] = unionLabels;
    node["depth"] = depth;
    node["expanded"] = false;
    node["children"] = QVariantList();
    
    return node;
}

void TopicIDLStruct::parseStructToTree(const QString &idlText)
{
    std::cout << "[TopicIDLStruct] ★★★ Starting IDL parsing ★★★" << std::endl;
    std::cout << "[TopicIDLStruct] Input text:\n" << idlText.toStdString() << std::endl;
    
    m_treeData.clear();
    
    if (idlText.isEmpty())
    {
        std::cout << "[TopicIDLStruct] Empty IDL text" << std::endl;
        emit treeDataChanged();
        return;
    }
    
    QStringList lines = idlText.split("\n", Qt::SkipEmptyParts);
    QString rootStructName;
    QVariantMap rootNode;
    QVariantList childNodes;
    int totalSize = 0;

    // Match "struct Name {" or "struct Namespace::Name {" or "struct Name"
    QRegularExpression structPattern(R"(struct\s+([\w:]+))");
    // Match "  TYPE NAME;" with optional leading spaces and optional semicolon
    QRegularExpression memberPattern(R"(^\s*(\w+)\s+(\w+)\s*;?\s*$)");
    
    bool inStruct = false;
    QString currentExtensibility = "FINAL";
    
    std::cout << "[TopicIDLStruct] Processing " << lines.size() << " lines" << std::endl;
    
    for (int lineNum = 0; lineNum < lines.size(); lineNum++)
    {
        const QString& line = lines[lineNum];
        QString trimmedLine = line.trimmed();
        
        std::cout << "[TopicIDLStruct] Line " << lineNum << ": '" << trimmedLine.toStdString() << "'" << std::endl;
        
        // Skip empty lines
        if (trimmedLine.isEmpty())
        {
            std::cout << "[TopicIDLStruct]   -> Empty line, skipping" << std::endl;
            continue;
        }
        
        // Check for extensibility annotation
        if (trimmedLine.startsWith("@"))
        {
            if (trimmedLine.contains("extensibility"))
            {
                currentExtensibility = "EXTENSIBLE";
                std::cout << "[TopicIDLStruct]   -> Found extensibility annotation" << std::endl;
            }
            continue;
        }
        
        // Check for struct start
        if (!inStruct)
        {
            QRegularExpressionMatch structMatch = structPattern.match(trimmedLine);
            if (structMatch.hasMatch())
            {
                rootStructName = structMatch.captured(1);
                
                // Remove namespace if present (Common1::struct1 -> struct1)
                if (rootStructName.contains("::"))
                {
                    QString originalName = rootStructName;
                    rootStructName = rootStructName.split("::").last();
                    std::cout << "[TopicIDLStruct]   -> Removed namespace: " << originalName.toStdString() 
                             << " -> " << rootStructName.toStdString() << std::endl;
                }
                
                inStruct = true;
                std::cout << "[TopicIDLStruct]   ✓ Found struct: " << rootStructName.toStdString() << std::endl;
                continue;
            }
        }
        
        // Check for struct end
        if (trimmedLine.startsWith("}") || trimmedLine.contains("};"))
        {
            std::cout << "[TopicIDLStruct]   -> Found struct end" << std::endl;
            inStruct = false;
            continue;
        }
        
        // Parse members (only when inside struct)
        if (inStruct)
        {
            // Skip lines containing only "{" or "}"
            if (trimmedLine == "{" || trimmedLine == "}")
            {
                std::cout << "[TopicIDLStruct]   -> Skipping brace-only line" << std::endl;
                continue;
            }
            
            QRegularExpressionMatch memberMatch = memberPattern.match(trimmedLine);
            if (memberMatch.hasMatch())
            {
                QString memberType = memberMatch.captured(1);
                QString memberName = memberMatch.captured(2);
                
                // Skip "struct" keyword lines
                if (memberType == "struct" || memberName == "}")
                {
                    std::cout << "[TopicIDLStruct]   -> Skipping struct keyword line" << std::endl;
                    continue;
                }
                
                // Calculate member size
                int memberSize = 4; // default
                
                if (memberType == "double") memberSize = 8;
                else if (memberType == "float") memberSize = 4;
                else if (memberType == "long") memberSize = 4;
                else if (memberType == "short") memberSize = 2;
                else if (memberType == "octet") memberSize = 1;
                else if (memberType == "char") memberSize = 1;
                else if (memberType == "string") memberSize = 256;
                else if (memberType == "boolean") memberSize = 1;
                else if (memberType == "unknown") memberSize = 1;
                else if (memberType.startsWith("struct")) memberSize = 64;
                else memberSize = 4;
                
                totalSize += memberSize;
                
                QVariantMap childNode = createTreeNode(
                    memberName,
                    memberType,
                    "",
                    "",
                    memberSize,
                    memberSize,
                    QString::number(memberSize),
                    QString::number(memberSize),
                    "",
                    1
                );
                
                childNodes.append(childNode);
                std::cout << "[TopicIDLStruct]   ✓ Added member: " << memberName.toStdString() 
                         << " (" << memberType.toStdString() << "), size=" << memberSize << std::endl;
            }
            else
            {
                std::cout << "[TopicIDLStruct]   ✗ Line did not match member pattern: '" 
                         << trimmedLine.toStdString() << "'" << std::endl;
            }
        }
    }
    
    // Create root node
    if (!rootStructName.isEmpty() && !childNodes.isEmpty())
    {
        int totalWithEncap = totalSize + 4;
        rootNode = createTreeNode(
            rootStructName,
            rootStructName,
            currentExtensibility,
            "",
            totalWithEncap,
            totalWithEncap,
            QString::number(totalSize + 2),
            QString::number(totalSize + 3),
            "",
            0
        );
        
        rootNode["children"] = childNodes;
        m_treeData.append(rootNode);
        
        std::cout << "[TopicIDLStruct] ★★★ SUCCESS! Root created with " << childNodes.size() 
                  << " children ★★★" << std::endl;
        std::cout << "[TopicIDLStruct] ★★★ m_treeData size: " << m_treeData.size() << " ★★★" << std::endl;
    }
    else
    {
        std::cout << "[TopicIDLStruct] ❌ ERROR: No root struct found or no members parsed!" << std::endl;
        std::cout << "[TopicIDLStruct]   rootStructName: '" << rootStructName.toStdString() << "'" << std::endl;
        std::cout << "[TopicIDLStruct]   childNodes.size(): " << childNodes.size() << std::endl;
    }
    
    emit treeDataChanged();
}
