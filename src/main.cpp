#include <QApplication>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QIcon>
#include <QQuickWindow>
#include <QDebug>
#include <iostream>

#include <indedds-monitor/Engine.h>
#include <indedds-monitor/model/mainwindow.h>
#include <indedds-monitor/model/dialog.h>
#include <indedds-monitor/model/widget.h>
#include <indedds-monitor/FileLoggerModel.h>
#include <indedds-monitor/StateControlModel.h>
#include <indedds-monitor/Controller.h>
#include <indedds-monitor/topic_idl_struct.h>
#include <indedds-monitor/model/system_overview_model.h>  // ADD THIS

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/resources/images/eprosima_logo.ico"));
    app.setOrganizationName("CDAC");
    app.setOrganizationDomain("cdac.in");
    app.setApplicationName("Admin & Monitor");

    /******************************************************************************************************************
     * Application engine                                                                                              *
     ******************************************************************************************************************/
    
    std::cout << "========================================" << std::endl;
    std::cout << "[Main] Creating Engine object..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    Engine engine;
    
    std::cout << "========================================" << std::endl;
    std::cout << "[Main] Engine created, calling enable()..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    QObject *topLevel = engine.enable();
    
    std::cout << "========================================" << std::endl;
    std::cout << "[Main] ✓ engine.enable() returned successfully" << std::endl;
    std::cout << "========================================" << std::endl;
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);

    if (!window)
    {
        qWarning("Error: Your root item has to be a Window.");
        return -1;
    }

    MainWindow mainWindow;
    Dialog dialog;
    Widget widget;
    FileLoggerModel fileLoggerModel;
    StateControlModel stateControlModel;
    
    // ========== CREATE SYSTEM OVERVIEW MODEL (AFTER ENGINE) ==========
    SystemOverviewModel* systemOverviewModel = new SystemOverviewModel(&app);
    std::cout << "[Main] ✓ SystemOverviewModel created" << std::endl;

    // ========== SET TOPIC IDL MODEL IN CONTROLLER ==========
    qDebug() << "[main] Setting topicIDLModel in controller...";
    TopicIDLStruct* topicIDLModel = engine.getTopicIDLModel();
    qDebug() << "[main] topicIDLModel pointer:" << topicIDLModel;

    Controller* controller = qobject_cast<Controller*>(
        engine.rootContext()->contextProperty("controller").value<QObject*>());
    
    if (controller && topicIDLModel) {
        controller->setTopicIDLModel(topicIDLModel);
        qDebug() << "[main] ✓ topicIDLModel set in controller successfully";
    } else {
        qWarning() << "[main] ✗ Failed to set topicIDLModel";
    }

    // ========== CONNECT ENGINE TO SYSTEM OVERVIEW MODEL ==========
    engine.setSystemOverviewModel(systemOverviewModel);
    std::cout << "[Main] ✓ SystemOverviewModel connected to Engine" << std::endl;

    // ========== REGISTER QML CONTEXT PROPERTIES ==========
    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);
    engine.rootContext()->setContextProperty("dialog", &dialog);
    engine.rootContext()->setContextProperty("widget", &widget);
    engine.rootContext()->setContextProperty("fileLoggerModel", &fileLoggerModel);
    engine.rootContext()->setContextProperty("stateControlModel", &stateControlModel);
    engine.rootContext()->setContextProperty("systemOverviewModel", systemOverviewModel);
    
    std::cout << "[Main] ✓ All QML context properties registered" << std::endl;
    
    // ========== CONNECT WIDGET TO MODEL ==========
    widget.setSystemOverviewModel(systemOverviewModel);
    std::cout << "[Main] ✓ Widget connected to SystemOverviewModel" << std::endl;
    
    engine.setWindow(&mainWindow);
    engine.setWidget(&widget);

    window->show();

    std::cout << "[Main] ✓✓✓ Application started successfully ✓✓✓" << std::endl;
    
    return app.exec();
}
