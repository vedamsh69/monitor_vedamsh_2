// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// This file is part of eProsima Fast DDS Monitor.
//
// eProsima Fast DDS Monitor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// eProsima Fast DDS Monitor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with eProsima Fast DDS Monitor. If not, see <https://www.gnu.org/licenses/>.

#include <QDebug>
#include <QStringBuilder>
#include <QtGlobal>
#include <vector>

#include <indedds-monitor/config.h>
#include <fastdds_statistics_backend/config.h>
#include <fastrtps/config.h>

#include <src/DL_subscriber/dloggerSubscriber.h>

#include <indedds-monitor/subscriber/HelloWorldSubscriber.h>
#include <indedds-monitor/publisher/HelloWorldPublisher.h>
#include <indedds-monitor/Controller.h>
#include <indedds-monitor/match/MatchAnalysesModel.h>
#include <indedds-monitor/Engine.h>
#include <indedds-monitor/utils.h>
#include <pthread.h>
#include <QTimer>
#include <thread>
#include <chrono>
// Admin Tool additions
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QCoreApplication>
#include <QStandardPaths>

namespace
{

    static QString shell_quote(const QString &s)
    {
        // safe single-quote for bash: ' -> '\'' (classic)
        QString out = s;
        out.replace("'", "'\\''");
        return "'" + out + "'";
    }

    enum class TerminalKind
    {
        XTerminalEmulator,
        GnomeTerminal,
        Konsole,
        Xfce4Terminal,
        XTerm,
        Unknown
    };

    struct TerminalSpec
    {
        QString program;
        TerminalKind kind{TerminalKind::Unknown};
    };

    static TerminalSpec find_terminal()
    {
        // "OS default" best-effort:
        // 1) Debian/Ubuntu: x-terminal-emulator (alternatives)
        // 2) GNOME/KDE/XFCE common terminals
        const struct Candidate
        {
            const char *name;
            TerminalKind kind;
        } candidates[] = {
            {"x-terminal-emulator", TerminalKind::XTerminalEmulator},
            {"gnome-terminal", TerminalKind::GnomeTerminal},
            {"konsole", TerminalKind::Konsole},
            {"xfce4-terminal", TerminalKind::Xfce4Terminal},
            {"xterm", TerminalKind::XTerm},
        };

        for (const auto &c : candidates)
        {
            const QString p = QStandardPaths::findExecutable(QString::fromUtf8(c.name));
            if (!p.isEmpty())
            {
                TerminalSpec spec;
                spec.program = p;
                spec.kind = c.kind;
                return spec;
            }
        }
        return TerminalSpec{};
    }

    static QStringList terminal_args(const TerminalSpec &term, const QString &bashCmd)
    {
        // We always run: bash -lc "<cmd>"
        // Keep/auto-close behavior is decided inside bashCmd itself.
        switch (term.kind)
        {
        case TerminalKind::GnomeTerminal:
            // gnome-terminal uses "--" before command
            return QStringList() << "--" << "bash" << "-lc" << bashCmd;

        case TerminalKind::Konsole:
            return QStringList() << "-e" << "bash" << "-lc" << bashCmd;

        case TerminalKind::Xfce4Terminal:
            // xfce4-terminal supports -e; keep it simple and compatible
            return QStringList() << "-e" << "bash" << "-lc" << bashCmd;

        case TerminalKind::XTerm:
            return QStringList() << "-e" << "bash" << "-lc" << bashCmd;

        case TerminalKind::XTerminalEmulator:
            // typically supports -e
            return QStringList() << "-e" << "bash" << "-lc" << bashCmd;

        default:
            // fallback: assume -e
            return QStringList() << "-e" << "bash" << "-lc" << bashCmd;
        }
    }

    static QString build_bash_command_cd_and_run(
        const QString &workDir,
        const QString &runCmd,
        bool keepOpenAfter)
    {
        // Run in a login-ish bash; we still enforce cd to selected dir.
        // keepOpenAfter: terminal stays open after command completes.
        QString cmd = "set -o pipefail; cd -- " + shell_quote(workDir) + " && " + runCmd;
        if (keepOpenAfter)
        {
            cmd += "; echo; echo '--- Command finished. Terminal kept open ---'; exec bash";
        }
        return cmd;
    }

    static QString start_terminal_detached(
        const QString &workDir,
        const QString &runCmd,
        bool keepOpenAfter)
    {
        TerminalSpec term = find_terminal();
        if (term.program.isEmpty())
        {
            return "No supported terminal found. Install one of: gnome-terminal / konsole / xfce4-terminal / xterm.";
        }

        const QString bashCmd = build_bash_command_cd_and_run(workDir, runCmd, keepOpenAfter);
        const QStringList args = terminal_args(term, bashCmd);

        const bool ok = QProcess::startDetached(term.program, args);
        if (!ok)
        {
            return "Failed to launch terminal process.";
        }
        return "";
    }
    static QString resolve_admin_work_dir()
    {
        // 1) Where app is launched from (current working directory)
        const QString cwd = QDir::currentPath();
        if (!cwd.isEmpty() && QDir(cwd).exists())
        {
            return cwd;
        }

        // 2) Folder of the executable
        const QString appDir = QCoreApplication::applicationDirPath();
        if (!appDir.isEmpty() && QDir(appDir).exists())
        {
            return appDir;
        }

        // 3) Home fallback
        return QDir::homePath();
    }

    static bool bash_command_exists_login_shell(const QString &cmdName)
    {
        // Use the same environment model as terminal execution: bash -lc
        QProcess p;
        p.start("bash", QStringList() << "-lc"
                                      << ("command -v " + shell_quote(cmdName) + " >/dev/null 2>&1"));

        if (!p.waitForFinished(2000))
        {
            // If check hangs for any reason, do not hard-block; allow attempt to run.
            p.kill();
            p.waitForFinished(500);
            return true;
        }
        return (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0);
    }

    static QString prepare_db_run_command(
        const QString &workDir,
        const QString &cmdName,
        QString *outRunCmd)
    {
        if (!outRunCmd)
        {
            return "Internal error: outRunCmd is null.";
        }

        // Prefer local executable in the directory where app is launched
        QFileInfo localFi(QDir(workDir).filePath(cmdName));
        if (localFi.exists() && localFi.isFile())
        {
            if (!localFi.isExecutable())
            {
                return "Command exists but is not executable:\n" + localFi.absoluteFilePath() +
                       "\n\nFix: chmod +x " + localFi.absoluteFilePath();
            }
            // Because we cd into workDir, execute as ./cmdName
            *outRunCmd = "./" + cmdName;
            return "";
        }

        // Otherwise rely on PATH (login shell)
        if (!bash_command_exists_login_shell(cmdName))
        {
            return "Command not found: " + cmdName + "\n\n"
                                                     "Searched:\n"
                                                     "• Local: " +
                   QDir(workDir).filePath(cmdName) + "\n"
                                                     "• PATH (bash -lc)\n\n"
                                                     "Fix options:\n"
                                                     "• Put '" +
                   cmdName + "' in the directory where the app is launched\n"
                             "  OR\n"
                             "• Add it to PATH for a login shell (e.g. ~/.bash_profile or /etc/profile)\n";
        }
        *outRunCmd = cmdName;
        return "";
    }

    static QString run_db_service_command(const QString &cmdName, bool keepOpenAfter)
    {
        const QString workDir = resolve_admin_work_dir();

        QString runCmd;
        const QString prepErr = prepare_db_run_command(workDir, cmdName, &runCmd);
        if (!prepErr.isEmpty())
        {
            return prepErr;
        }

        // Execute in terminal from resolved directory
        return start_terminal_detached(workDir, runCmd, keepOpenAfter);
    }
} // namespace

QString Controller::db_start_database_service()
{
    // Start should generally complete and exit; auto-close terminal
    return run_db_service_command("start_db", /*keepOpenAfter=*/false);
}

QString Controller::db_launch_database_service()
{
    // Launch is likely long-running; keep terminal open
    return run_db_service_command("launch_db", /*keepOpenAfter=*/true);
}

QString Controller::db_stop_database_service()
{
    // Stop should generally complete and exit; auto-close terminal
    return run_db_service_command("stop_db", /*keepOpenAfter=*/false);
}

QString Controller::launch_rnr_controller()
{
    const QString home = QDir::homePath();

    // -----------------------------------------------------------------------
    // Step 1: Locate the actual Python script (bypass the colcon wrapper).
    // The wrapper unconditionally overwrites PYTHONPATH — we run python3
    // directly on the .py script so we control the environment fully.
    // -----------------------------------------------------------------------
    const QStringList pyScriptCandidates = QStringList()
                                           << (home + "/indedds/install/src/ddsrecorder_controller.py")
                                           << (home + "/indedds/install/lib/ddsrecorder_controller.py")
                                           << (home + "/indedds/install/bin/ddsrecorder_controller.py")
                                           << (home + "/iindedds/install/src/ddsrecorder_controller.py");

    QString pyScript;
    for (const QString &c : pyScriptCandidates)
    {
        if (QFileInfo::exists(c))
        {
            pyScript = c;
            break;
        }
    }

    // -----------------------------------------------------------------------
    // Step 2: Fallback to wrapper exe only if .py script not found.
    // -----------------------------------------------------------------------
    QString wrapperExe;
    if (pyScript.isEmpty())
    {
        wrapperExe = QStandardPaths::findExecutable("ddsrecorder_controller");
        if (wrapperExe.isEmpty())
        {
            const QStringList wrapperCandidates = QStringList()
                                                  << (home + "/indedds/install/bin/ddsrecorder_controller")
                                                  << (home + "/iindedds/install/bin/ddsrecorder_controller");

            for (const QString &c : wrapperCandidates)
            {
                QFileInfo fi(c);
                if (fi.exists() && fi.isFile())
                {
                    wrapperExe = c;
                    break;
                }
            }
        }

        if (wrapperExe.isEmpty())
        {
            return "ddsrecorder_controller not found.\n\n"
                   "Searched for Python script at:\n"
                   "  ~/indedds/install/src/ddsrecorder_controller.py\n"
                   "  ~/indedds/install/lib/ddsrecorder_controller.py\n"
                   "  ~/indedds/install/bin/ddsrecorder_controller.py\n\n"
                   "Searched for wrapper executable at:\n"
                   "  ~/indedds/install/bin/ddsrecorder_controller\n"
                   "  ~/iindedds/install/bin/ddsrecorder_controller\n"
                   "  (also searched PATH)";
        }
    }

    // -----------------------------------------------------------------------
    // Step 3: Build the bash command.
    //
    // We do NOT source setup.bash. Instead we set LD_LIBRARY_PATH and
    // PYTHONPATH directly from the known install layout — no side-effects.
    //
    // CONFIRMED REQUIRED PATHS (proven by diagnostic logs):
    //
    //   LD_LIBRARY_PATH:
    //     ~/indedds/install/lib            — fastdds .so and other DDS libs
    //
    //   PYTHONPATH (in this exact order):
    //     ~/indedds/install/src            — ControllerGUI.py, Controller.py,
    //                                        Logger.py
    //     ~/indedds/install/lib/python3.12/site-packages
    //                                      — DdsRecorderCommand.py,
    //                                        DdsRecorderStatus.py
    //     ~/indedds/install/lib            — fastdds.py
    //     /usr/lib/python3/dist-packages   — PyQt5 (Ubuntu)
    //     /usr/lib/python3.12/dist-packages
    //     /usr/local/lib/python3.12/dist-packages
    //     /usr/lib/python3/site-packages   — RHEL 8 / AlmaLinux
    //     /usr/lib/python3.12/site-packages
    //
    //   PYTHONNOUSERSITE=1:
    //     Prevents Python auto-adding ~/.local/lib/python3.12/site-packages
    //     which contains a broken enum34 backport that shadows stdlib enum,
    //     causing: AttributeError: module 'enum' has no attribute 'global_enum'
    //     which breaks the entire PyQt5 import chain.
    // -----------------------------------------------------------------------
    QString runCmd;

    // LD_LIBRARY_PATH — DDS shared libraries (.so files)
    runCmd += "export LD_LIBRARY_PATH="
              "\"" +
              home + "/indedds/install/lib"
                     ":${LD_LIBRARY_PATH}\"; ";

    // Block broken ~/.local enum backport from shadowing stdlib enum.
    runCmd += "export PYTHONNOUSERSITE=1; ";

    // PYTHONPATH — exact paths proven by working terminal diagnostic.
    // python3.12 version is hardcoded because that is what this system runs.
    // The version-independent /usr/lib/python3/* paths handle RHEL/Alma too.
    runCmd +=
        "export PYTHONPATH="
        "\"" +
        home + "/indedds/install/src"
               ":" +
        home + "/indedds/install/lib/python3.12/site-packages"
               ":" +
        home + "/indedds/install/lib"
               ":/usr/lib/python3/dist-packages"
               ":/usr/lib/python3.12/dist-packages"
               ":/usr/local/lib/python3.12/dist-packages"
               ":/usr/lib/python3/site-packages"
               ":/usr/lib/python3.12/site-packages"
               "\"; ";

    // PRIMARY:  run python3 <script> directly — full control of environment.
    // FALLBACK: run wrapper exe (only when .py not found).
    if (!pyScript.isEmpty())
        runCmd += "python3 " + shell_quote(pyScript);
    else
        runCmd += shell_quote(wrapperExe);

    runCmd += " || { echo; echo 'Failed to start RnR Controller'; exec bash; }";

    return start_terminal_detached(home, runCmd, /*keepOpenAfter=*/true);
}

// ========================================
// CONSTRUCTOR
// ========================================
Controller::Controller(Engine *engine, QObject *parent)
    : QObject(parent), engine_(engine), topicIDLModel_(nullptr), match_analyses_model_(nullptr)
{
    qDebug() << "[Controller] Constructor called";
    qDebug() << "[Controller] Engine:" << engine_;
    qDebug() << "[Controller] topicIDLModel_ initialized to nullptr";
    qDebug() << "[Controller] match_analyses_model_ initialized to nullptr";
}

void Controller::init_monitor(
    int domain)
{
    std::cout << "[DEBUG] Entering Controller::init_monitor(int)" << std::endl;
    std::cout << "[DEBUG] domain = " << domain << std::endl;

    engine_->init_monitor(domain);

    std::cout << "[DEBUG] Exiting Controller::init_monitor(int)" << std::endl;
}

void Controller::init_monitor(
    QString discovery_server_guid_prefix,
    QString discovery_server_locators)
{
    std::cout << "[DEBUG] Entering Controller::init_monitor(QString, QString)" << std::endl;
    std::cout << "[DEBUG] discovery_server_guid_prefix = " << discovery_server_guid_prefix.toStdString()
              << ", discovery_server_locators = " << discovery_server_locators.toStdString() << std::endl;

    engine_->init_monitor(discovery_server_guid_prefix, discovery_server_locators);

    std::cout << "[DEBUG] Exiting Controller::init_monitor(QString, QString)" << std::endl;
}

void Controller::host_click(
    QString id)
{
    std::cout << "[DEBUG] Entering Controller::host_click" << std::endl;
    std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

    auto backend_id = backend::models_id_to_backend_id(id);
    std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

    engine_->entity_clicked(backend_id, backend::EntityKind::HOST);

    std::cout << "[DEBUG] Exiting Controller::host_click" << std::endl;
}

void Controller::user_click(
    QString id)
{
    std::cout << "[DEBUG] Entering Controller::user_click" << std::endl;
    std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

    auto backend_id = backend::models_id_to_backend_id(id);
    std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

    engine_->entity_clicked(backend_id, backend::EntityKind::USER);

    std::cout << "[DEBUG] Exiting Controller::user_click" << std::endl;
}

void Controller::process_click(
    QString id)
{
    std::cout << "[DEBUG] Entering Controller::process_click" << std::endl;
    std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

    auto backend_id = backend::models_id_to_backend_id(id);
    std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

    engine_->entity_clicked(backend_id, backend::EntityKind::PROCESS);

    std::cout << "[DEBUG] Exiting Controller::process_click" << std::endl;
}

// void Controller::domain_click(
//     QString id)
// {
//     std::cout << "[DEBUG] Entering Controller::domain_click" << std::endl;
//     std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

//     auto backend_id = backend::models_id_to_backend_id(id);
//     std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

//     engine_->entity_clicked(backend_id, backend::EntityKind::DOMAIN);

//     std::cout << "[DEBUG] Exiting Controller::domain_click" << std::endl;
// }

void Controller::domain_click(
    QString id)
{
    try
    {
        std::cout << "[DEBUG] Entering Controller::domain_click" << std::endl;
        std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

        auto backend_id = backend::models_id_to_backend_id(id);
        std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

        engine_->entity_clicked(backend_id, backend::EntityKind::DOMAIN);

        std::cout << "[DEBUG] Exiting Controller::domain_click" << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[ERROR] domain_click failed with exception: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "[ERROR] domain_click failed with unknown exception." << std::endl;
    }
}

void Controller::topic_click(
    QString id)
{
    std::cout << "[DEBUG] Entering Controller::topic_click" << std::endl;
    std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

    auto backend_id = backend::models_id_to_backend_id(id);
    std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

    engine_->entity_clicked(backend_id, backend::EntityKind::TOPIC);

    std::cout << "[DEBUG] Exiting Controller::topic_click" << std::endl;
}

void Controller::participant_click(
    QString id)
{
    std::cout << "[DEBUG] Entering Controller::participant_click" << std::endl;
    std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

    auto backend_id = backend::models_id_to_backend_id(id);
    std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;

    engine_->entity_clicked(backend_id, backend::EntityKind::PARTICIPANT);

    std::cout << "[DEBUG] Exiting Controller::participant_click" << std::endl;
}

void Controller::endpoint_click(
    QString id)
{
    std::cout << "[DEBUG] Entering Controller::endpoint_click" << std::endl;
    std::cout << "[DEBUG] id = " << id.toStdString() << std::endl;

    auto backend_id = backend::models_id_to_backend_id(id);
    std::cout << "[DEBUG] Converted id to backend_id = " << backend_id << std::endl;
    std::cout << "[DEBUG] WARNING: EntityKind assumed as DATAWRITER (unknown if DataWriter or DataReader)" << std::endl;

    // WARNING: we do not know if it is DataWriter or DataReader
    engine_->entity_clicked(backend_id, backend::EntityKind::DATAWRITER);

    std::cout << "[DEBUG] Exiting Controller::endpoint_click" << std::endl;
}

void Controller::locator_click(
    QString id)
{
    std::cout << "[DEBUG] Controller::locator_click called with id=" << id.toStdString() << std::endl;
    engine_->entity_clicked(backend::models_id_to_backend_id(id), backend::EntityKind::LOCATOR);
    std::cout << "[DEBUG] Controller::locator_click finished for id=" << id.toStdString() << std::endl;
}

void Controller::update_available_entity_ids(
    QString entity_kind,
    QString entity_model_id)
{
    std::cout << "[DEBUG] Controller::update_available_entity_ids called with entity_kind="
              << entity_kind.toStdString() << ", entity_model_id=" << entity_model_id.toStdString() << std::endl;

    engine_->on_selected_entity_kind(backend::string_to_entity_kind(entity_kind), entity_model_id);

    std::cout << "[DEBUG] Controller::update_available_entity_ids finished" << std::endl;
}

void Controller::refresh_click()
{
    std::cout << "[DEBUG] Controller::refresh_click called" << std::endl;
    engine_->refresh_engine();
    std::cout << "[DEBUG] Controller::refresh_click finished" << std::endl;
}

void Controller::clear_entities()
{
    std::cout << "[DEBUG] Controller::clear_entities called" << std::endl;
    engine_->clear_entities();
    std::cout << "[DEBUG] Controller::clear_entities finished" << std::endl;
}

void Controller::clear_statistics_data(
    quint64 time_to)
{
    std::cout << "[DEBUG] Controller::clear_statistics_data called with time_to=" << time_to << std::endl;
    engine_->clear_statistics_data(time_to);
    std::cout << "[DEBUG] Controller::clear_statistics_data finished" << std::endl;
}

void Controller::clear_log()
{
    std::cout << "[DEBUG] Controller::clear_log called" << std::endl;
    engine_->clear_log();
    std::cout << "[DEBUG] Controller::clear_log finished" << std::endl;
}

void Controller::clear_issues()
{
    std::cout << "[DEBUG] Controller::clear_issues called" << std::endl;
    engine_->clear_issues();
    std::cout << "[DEBUG] Controller::clear_issues finished" << std::endl;
}

qreal Controller::get_max_real()
{
    std::cout << "[DEBUG] Controller::get_max_real called" << std::endl;
    qreal value = std::numeric_limits<qreal>::max();
    std::cout << "[DEBUG] Controller::get_max_real returning " << value << std::endl;
    return value;
}

qreal Controller::get_min_real()
{
    std::cout << "[DEBUG] Controller::get_min_real called" << std::endl;
    qreal value = std::numeric_limits<qreal>::lowest();
    std::cout << "[DEBUG] Controller::get_min_real returning " << value << std::endl;
    return value;
}

quint64 Controller::get_max_uint()
{
    qDebug() << "Entering get_max_uint()";
    quint64 value = std::numeric_limits<quint64>::max();
    qDebug() << "get_max_uint returning:" << value;
    return value;
}

quint64 Controller::get_min_uint()
{
    qDebug() << "Entering get_min_uint()";
    quint64 value = std::numeric_limits<quint64>::min();
    qDebug() << "get_min_uint returning:" << value;
    return value;
}

QtCharts::QVXYModelMapper *Controller::add_statistics_data(
    quint64 chartbox_id,
    QString data_kind,
    QString source_entity_id,
    QString target_entity_id,
    quint16 bins,
    quint64 start_time,
    bool start_time_default,
    quint64 end_time,
    bool end_time_default,
    QString statistic_kind)
{
    qDebug() << "Entering add_statistics_data()";
    qDebug() << "chartbox_id:" << chartbox_id;
    qDebug() << "data_kind:" << data_kind;
    qDebug() << "source_entity_id:" << source_entity_id;
    qDebug() << "target_entity_id:" << target_entity_id;
    qDebug() << "bins:" << bins;
    qDebug() << "start_time:" << start_time << " (default:" << start_time_default << ")";
    qDebug() << "end_time:" << end_time << " (default:" << end_time_default << ")";
    qDebug() << "statistic_kind:" << statistic_kind;

    auto result = engine_->on_add_statistics_data_series(
        chartbox_id,
        backend::string_to_data_kind(data_kind),
        backend::models_id_to_backend_id(source_entity_id),
        backend::models_id_to_backend_id(target_entity_id),
        bins,
        start_time,
        start_time_default,
        end_time,
        end_time_default,
        backend::string_to_statistic_kind(statistic_kind));

    qDebug() << "Exiting add_statistics_data(), returning QVXYModelMapper*:" << result;
    return result;
}

QString Controller::fastdds_version()
{
    qDebug() << "Fetching fastdds_version";
    QString value = utils::to_QString(FASTRTPS_VERSION_STR);
    qDebug() << "fastdds_version:" << value;
    return value;
}

QString Controller::fastdds_statistics_backend_version()
{
    qDebug() << "Fetching fastdds_statistics_backend_version";
    QString value = utils::to_QString(FASTDDS_STATISTICS_BACKEND_VERSION_STR);
    qDebug() << "fastdds_statistics_backend_version:" << value;
    return value;
}

QString Controller::qt_version()
{
    qDebug() << "Fetching qt_version";
    QString value = utils::to_QString(qVersion());
    qDebug() << "qt_version:" << value;
    return value;
}

QString Controller::fastdds_monitor_version()
{
    qDebug() << "Fetching fastdds_monitor_version";
    QString value = utils::to_QString(FASTDDS_MONITOR_VERSION_STR);
    qDebug() << "fastdds_monitor_version:" << value;
    return value;
}

QString Controller::system_info()
{
    qDebug() << "Fetching system_info";
    QString value = utils::to_QString(SYSTEM_NAME) % " " %
                    utils::to_QString(SYSTEM_PROCESSOR) % " " %
                    utils::to_QString(SYSTEM_VERSION);
    qDebug() << "system_info:" << value;
    return value;
}

QString Controller::build_date()
{
    qDebug() << "Fetching build_date";
    QString value = utils::to_QString(BUILD_DATE);
    qDebug() << "build_date:" << value;
    return value;
}

QString Controller::git_commit()
{
    qDebug() << "Fetching git_commit";
    QString value = utils::to_QString(GIT_COMMIT_HASH);
    qDebug() << "git_commit:" << value;
    return value;
}

bool Controller::inactive_visible()
{
    qDebug() << "Checking inactive_visible";
    bool value = engine_->inactive_visible();
    qDebug() << "inactive_visible:" << value;
    return value;
}

void Controller::change_inactive_visible()
{
    qDebug() << "Controller::change_inactive_visible() called";
    engine_->change_inactive_visible();
    qDebug() << "Controller::change_inactive_visible() finished";
}

bool Controller::metatraffic_visible()
{
    qDebug() << "Controller::metatraffic_visible() called";
    bool result = engine_->metatraffic_visible();
    qDebug() << "Controller::metatraffic_visible() returned:" << result;
    return result;
}

void Controller::change_metatraffic_visible()
{
    qDebug() << "Controller::change_metatraffic_visible() called";
    engine_->change_metatraffic_visible();
    qDebug() << "Controller::change_metatraffic_visible() finished";
}

void Controller::refresh_summary()
{
    qDebug() << "Controller::refresh_summary() called";
    engine_->refresh_summary();
    qDebug() << "Controller::refresh_summary() finished";
}

void Controller::send_error(
    QString error_msg,
    ErrorType error_type /*= GENERIC*/)
{
    qDebug() << "Controller::send_error() called with error_msg:" << error_msg
             << " error_type:" << static_cast<int>(error_type);
    // Must convert enumeration to int in order to qml understand it
    emit error(error_msg, static_cast<typename std::underlying_type<ErrorType>::type>(error_type));
    qDebug() << "Controller::send_error() emitted signal";
}

void Controller::update_dynamic_chartbox(
    quint64 chartbox_id,
    quint64 time_to)
{
    qDebug() << "Controller::update_dynamic_chartbox() called with chartbox_id:" << chartbox_id
             << " time_to:" << time_to;
    engine_->update_dynamic_chartbox(chartbox_id, time_to);
    qDebug() << "Controller::update_dynamic_chartbox() finished";
}

void Controller::set_alias(
    QString entity_id,
    QString new_alias,
    QString entity_kind)
{
    qDebug() << "Controller::set_alias() called with entity_id:" << entity_id
             << " new_alias:" << new_alias
             << " entity_kind:" << entity_kind;
    engine_->set_alias(
        backend::models_id_to_backend_id(entity_id),
        utils::to_string(new_alias),
        backend::string_to_entity_kind(entity_kind));
    qDebug() << "Controller::set_alias() finished";
}

QString Controller::get_data_kind_units(
    QString data_kind)
{
    qDebug() << "Controller::get_data_kind_units() called with data_kind:" << data_kind;
    QString result = utils::to_QString(engine_->get_data_kind_units(data_kind));
    qDebug() << "Controller::get_data_kind_units() returning:" << result;
    return result;
}

void Controller::save_csv(
    QString file_name,
    QList<quint64> chartbox_ids,
    QList<quint64> series_indexes,
    QStringList data_kinds,
    QStringList chartbox_names,
    QStringList label_names)
{
    qDebug() << "Controller::save_csv() called with file_name:" << file_name
             << " chartbox_ids:" << chartbox_ids
             << " series_indexes:" << series_indexes
             << " data_kinds:" << data_kinds
             << " chartbox_names:" << chartbox_names
             << " label_names:" << label_names;
    engine_->save_csv(
        file_name,
        chartbox_ids,
        series_indexes,
        data_kinds,
        chartbox_names,
        label_names);
    qDebug() << "Controller::save_csv() finished";
}

void Controller::dump(
    QString file_name,
    bool clear)
{
    qDebug() << "[Controller::dump] Called with file_name:" << file_name << "clear:" << clear;
    engine_->dump(
        file_name,
        clear);
    qDebug() << "[Controller::dump] Finished dumping";
}

QStringList Controller::ds_supported_transports()
{
    qDebug() << "[Controller::ds_supported_transports] Called";
    QStringList transports = utils::to_QStringList(engine_->ds_supported_transports());
    qDebug() << "[Controller::ds_supported_transports] Returning transports:" << transports;
    return transports;
}

QStringList Controller::get_statistic_kinds()
{
    qDebug() << "[Controller::get_statistic_kinds] Called";
    QStringList kinds = utils::to_QStringList(engine_->get_statistic_kinds());
    qDebug() << "[Controller::get_statistic_kinds] Returning kinds:" << kinds;
    return kinds;
}

QStringList Controller::get_data_kinds()
{
    qDebug() << "[Controller::get_data_kinds] Called";
    QStringList dataKinds = utils::to_QStringList(engine_->get_data_kinds());
    qDebug() << "[Controller::get_data_kinds] Returning data kinds:" << dataKinds;
    return dataKinds;
}

bool Controller::data_kind_has_target(
    const QString &data_kind)
{
    qDebug() << "[Controller::data_kind_has_target] Called with data_kind:" << data_kind;
    bool result = engine_->data_kind_has_target(data_kind);
    qDebug() << "[Controller::data_kind_has_target] Result:" << result;
    return result;
}

void Controller::change_max_points(
    quint64 chartbox_id,
    quint64 series_id,
    quint64 new_max_point)
{
    qDebug() << "[Controller::change_max_points] Called with chartbox_id:" << chartbox_id
             << "series_id:" << series_id
             << "new_max_point:" << new_max_point;
    engine_->change_max_points(chartbox_id, series_id, new_max_point);
    qDebug() << "[Controller::change_max_points] Finished updating max points";
}

QString Controller::get_domain_view_graph(
    QString entity_id)
{
    qDebug() << "[Controller::get_domain_view_graph] Called with entity_id:" << entity_id;
    backend::Graph domain_view = engine_->get_domain_view_graph(backend::models_id_to_backend_id(entity_id));
    QString graphStr = QString::fromUtf8(domain_view.dump().data(), int(domain_view.dump().size()));
    qDebug() << "[Controller::get_domain_view_graph] Returning graph string of size:" << graphStr.size();
    return graphStr;
}

void *Controller::create_subscriber(void *arg)
{
    qDebug() << "[Controller::create_subscriber] ========== THREAD START ==========";
    qDebug() << "[Controller::create_subscriber] Thread entry point called";
    qDebug() << "[Controller::create_subscriber] Argument pointer:" << arg;

    // ========== CRITICAL FIX: arg is now HelloWorldSubscriber*, not TopicIDLStruct* ==========
    HelloWorldSubscriber *subscriber = static_cast<HelloWorldSubscriber *>(arg);

    if (!subscriber)
    {
        qCritical() << "[Controller::create_subscriber] ERROR: NULL subscriber pointer!";
        return nullptr;
    }

    qDebug() << "[Controller::create_subscriber] Subscriber object at:" << subscriber;
    qDebug() << "[Controller::create_subscriber] Calling subscriber->run()...";

    // Run the subscriber (this will block until stop() is called)
    subscriber->run();

    qDebug() << "[Controller::create_subscriber] subscriber->run() returned";
    qDebug() << "[Controller::create_subscriber] Thread exiting gracefully";
    qDebug() << "[Controller::create_subscriber] ========== THREAD END ==========";

    return nullptr;
}

void *Controller::create_publisher(void *arg)
{
    qDebug() << "[Controller::create_publisher] Called with arg pointer:" << arg;
    TopicIDLStruct *topicIDLModel = static_cast<TopicIDLStruct *>(arg);
    qDebug() << "[Controller::create_publisher] Extracted topic name:" << topicIDLModel->topicName();

    std::string topic_name = std::string(topicIDLModel->topicName().toStdString());
    HelloWorldPublisher mypub;

    qDebug() << "[Controller::create_publisher] Initializing publisher for topic:" << QString::fromStdString(topic_name);
    if (mypub.init(topic_name, 0, topicIDLModel))
    {
        qDebug() << "[Controller::create_publisher] Publisher init successful, running...";
        mypub.run(10); // Publish 10 samples
        qDebug() << "[Controller::create_publisher] Publisher run finished";
    }
    else
    {
        qDebug() << "[Controller::create_publisher] Publisher init failed!";
    }

    qDebug() << "[Controller::create_publisher] Returning nullptr";
    return nullptr;
}

void Controller::startDynamicSubscriber(const QString &topicName)
{
    qDebug() << "[Controller::startDynamicSubscriber] ========== START ==========";
    qDebug() << "[Controller::startDynamicSubscriber] Called with topicName:" << topicName;

    TopicIDLStruct *topicIDLModel = engine_->getTopicIDLModel();

    if (!topicIDLModel)
    {
        qCritical() << "[Controller::startDynamicSubscriber] ERROR: topicIDLModel is NULL!";
        return;
    }

    qDebug() << "[Controller::startDynamicSubscriber] Got TopicIDLModel at" << topicIDLModel;

    int domainId = 0; // TODO: Get actual domain ID
    qDebug() << "[Controller::startDynamicSubscriber] Using Domain ID:" << domainId;

    QPair<int, QString> key = qMakePair(domainId, topicName);

    if (activeSubscriptions_.contains(key))
    {
        qWarning() << "[Controller::startDynamicSubscriber] Already subscribed!";
        return;
    }

    qDebug() << "[Controller::startDynamicSubscriber] Creating new subscriber...";

    topicIDLModel->setTopicName(topicName);

    // ========== CREATE SUBSCRIBER ==========
    HelloWorldSubscriber *subscriber = new HelloWorldSubscriber();
    std::string topic_name_std = topicName.toStdString();

    qDebug() << "[Controller::startDynamicSubscriber] Subscriber created at:" << subscriber;
    qDebug() << "[Controller::startDynamicSubscriber] Initializing subscriber...";

    if (!subscriber->init(topic_name_std, topicIDLModel))
    {
        qCritical() << "[Controller::startDynamicSubscriber] Subscriber init failed!";
        delete subscriber;
        return;
    }

    qDebug() << "[Controller::startDynamicSubscriber] ✓ Subscriber initialized successfully";

    // ========== STORE SUBSCRIBER BEFORE THREAD ==========
    subscriberMap_[key] = subscriber;
    activeSubscriptions_.insert(key);

    qDebug() << "[Controller::startDynamicSubscriber] Subscriber stored in maps";
    qDebug() << "[Controller::startDynamicSubscriber] Creating pthread...";

    // ========== CRITICAL FIX: Pass subscriber pointer, NOT topicIDLModel! ==========
    pthread_t thread;
    int result = pthread_create(&thread, NULL,
                                Controller::create_subscriber,
                                subscriber); // ← PASS SUBSCRIBER, NOT topicIDLModel!

    if (result == 0)
    {
        qDebug() << "[Controller::startDynamicSubscriber] ✓ Thread created successfully";
        qDebug() << "[Controller::startDynamicSubscriber] Thread ID:" << thread;

        // Store thread handle
        subscriberThreads_[key] = thread;

        // Detach thread
        pthread_detach(thread);
        qDebug() << "[Controller::startDynamicSubscriber] Thread detached";
    }
    else
    {
        qCritical() << "[Controller::startDynamicSubscriber] ✗ Thread creation failed:" << result;
        delete subscriber;
        subscriberMap_.remove(key);
        activeSubscriptions_.remove(key);
    }

    qDebug() << "[Controller::startDynamicSubscriber] ========== END ==========";
}

void Controller::startDynamicPublisher(const QString &topicName)
{
    qDebug() << "[Controller::startDynamicPublisher] Called with topicName:" << topicName;

    TopicIDLStruct *topicIDLModel = engine_->getTopicIDLModel();
    qDebug() << "[Controller::startDynamicPublisher] Got TopicIDLModel at:" << topicIDLModel;

    topicIDLModel->setTopicName(topicName);
    qDebug() << "[Controller::startDynamicPublisher] Topic name set to:" << topicIDLModel->topicName();

    int result = pthread_create(&pubThread, NULL, &Controller::create_publisher, topicIDLModel);
    if (result == 0)
    {
        qDebug() << "[Controller::startDynamicPublisher] Publisher thread created successfully";
    }
    else
    {
        qDebug() << "[Controller::startDynamicPublisher] Failed to create publisher thread, error code:" << result;
    }
}

void *Controller::create_dl_subscriber(void *arg)
{
    std::cout << "[DEBUG] Entered Controller::create_dl_subscriber" << std::endl;
    std::cout << "[DEBUG] Argument passed to create_dl_subscriber: " << arg << std::endl;

    (void)arg; // Suppress unused parameter warning

    std::cout << "[DEBUG] Creating dloggerSubscriber object..." << std::endl;
    dloggerSubscriber mysub2;

    std::cout << "[DEBUG] Calling dloggerSubscriber::init()" << std::endl;
    if (mysub2.init())
    {
        std::cout << "[DEBUG] dloggerSubscriber::init() returned true" << std::endl;
        std::cout << "[DEBUG] Calling dloggerSubscriber::run()" << std::endl;
        mysub2.run();
        std::cout << "[DEBUG] Finished dloggerSubscriber::run()" << std::endl;
    }
    else
    {
        std::cout << "[DEBUG] dloggerSubscriber::init() returned false. Subscriber not started." << std::endl;
    }

    std::cout << "[DEBUG] Exiting Controller::create_dl_subscriber" << std::endl;
    return nullptr;
}

void Controller::startDynamicDLSubscriber()
{
    std::cout << "[DEBUG] Entered Controller::startDynamicDLSubscriber" << std::endl;

    int result = pthread_create(&dlSubThread, nullptr, &Controller::create_dl_subscriber, nullptr);
    if (result == 0)
    {
        std::cout << "[DEBUG] pthread_create for dlSubThread succeeded." << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] pthread_create for dlSubThread failed with error code: " << result << std::endl;
    }

    std::cout << "[DEBUG] Exiting Controller::startDynamicDLSubscriber" << std::endl;
}

// Add this implementation at the end of Controller.cpp

PublicationManager *Controller::createPublicationManager()
{
    qDebug() << "[Controller] Creating new PublicationManager";

    PublicationManager *manager = new PublicationManager(this);
    m_publicationManagers.append(manager);

    emit publicationCreated(manager);

    return manager;
}

void Controller::startPublisherWithDiscovery(const QString& topicName, int domainId)
{
    qDebug() << "[Controller::startPublisherWithDiscovery] topic=" << topicName
            << "domain=" << domainId;

    if (!topicIDLModel_)
    {
        qWarning() << "[Controller] topicIDLModel_ is NULL!";
        emit publisherDiscoveryFailed(topicName, "Internal error: TopicIDLModel not set");
        return;
    }

    QPair<int, QString> key = qMakePair(domainId, topicName);

    if (m_pendingDiscovery_.contains(key))
    {
        qWarning() << "[Controller] Discovery already in progress for"
                   << topicName << "domain" << domainId << "— ignoring duplicate request";
        return;
    }

    // If a ready publisher already exists for this (domain, topic) pair,
    // just fire the success signal immediately — no need to redo discovery.
    if (m_publisherMap.contains(key) && m_publisherMap[key] && m_publisherMap[key]->isReady())
    {
        qDebug() << "[Controller] Publisher already ready — emitting discovered immediately";
        emit publisherTypeDiscovered(topicName);
        return;
    }

    // Clean up any stale/unready publisher from a previous failed attempt
    if (m_publisherMap.contains(key))
    {
        qDebug() << "[Controller] Removing stale publisher for key";
        delete m_publisherMap[key];
        m_publisherMap.remove(key);
    }

    // Reset IDL model for fresh discovery
    topicIDLModel_->setTextData("");
    topicIDLModel_->setTopicName(topicName);

    // ── Create subscriber (for IDL text → editor display) ─────────────────
    HelloWorldSubscriber* subscriber = new HelloWorldSubscriber();
    if (!subscriber->init(topicName.toStdString(), topicIDLModel_))
    {
        qCritical() << "[Controller] Failed to init discovery subscriber";
        emit publisherDiscoveryFailed(topicName, "Failed to create type-discovery subscriber");
        delete subscriber;
        return;
    }

    // ── Create publisher (DDS entity creation + TypeLookup) ───────────────
    // init() only creates the DomainParticipant and starts TypeLookup; it does
    // NOT block.  The actual DataWriter is created later in ensureInitialized().
    HelloWorldPublisher* publisher = new HelloWorldPublisher();
    if (!publisher->init(topicName.toStdString(), domainId, topicIDLModel_))
    {
        qCritical() << "[Controller] Failed to init publisher participant";
        emit publisherDiscoveryFailed(topicName, "Failed to create publisher DomainParticipant");
        delete publisher;
        delete subscriber;
        return;
    }
    m_pendingDiscovery_.insert(key);
    qDebug() << "[Controller] Subscriber + Publisher created, launching discovery thread";

    // ── Background thread ─────────────────────────────────────────────────
    // We never block the Qt main thread here.  All slow operations run below.
    std::thread discovery_thread([this, topicName, domainId, key, subscriber, publisher]()
    {
        qDebug() << "[Controller] Discovery thread started";

        // Step 1 — Run subscriber to populate topicIDLModel->textData()
        // (blocks up to 5 s; also gives the publisher participant 5 s of
        //  network time so TypeLookup often completes during this wait)
        subscriber->runWithTimeout(12);
        QString idlText = topicIDLModel_->textData();

        // Dynamic endpoints can take extra time to provide type information.
        // Keep polling briefly before giving up, but only for our selected topic.
        if (idlText.trimmed().isEmpty())
        {
            for (int wait_ms = 0; wait_ms < 10000; wait_ms += 500)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (topicIDLModel_->topicName() == topicName)
                {
                    idlText = topicIDLModel_->textData();
                    if (!idlText.trimmed().isEmpty())
                    {
                        qDebug() << "[Controller] IDL arrived after extra wait:" << wait_ms + 500 << "ms";
                        break;
                    }
                }
            }
        }
        qDebug() << "[Controller] Subscriber done. IDL text length=" << idlText.length();

        // Step 2 — Wait for publisher TypeLookup (PRIMARY path)
        // If it already fired during the subscriber's 5 s window, this
        // returns immediately via the isReady() fast path.
        bool publisherReady = publisher->ensureInitialized(10000);
        qDebug() << "[Controller] ensureInitialized returned:" << publisherReady;

        // Step 3 — Fallback: build type from IDL text if TypeLookup timed out
        // (handles the case where no other DataWriter exists on the network,
        //  but a DataReader with TypeInformation was discovered by subscriber)
        if (!publisherReady && !idlText.trimmed().isEmpty())
        {
            qDebug() << "[Controller] TypeLookup failed — trying IDL text fallback";
            publisherReady = publisher->initializeFromIDLText(idlText);
            qDebug() << "[Controller] initializeFromIDLText returned:" << publisherReady;
        }

        // Step 3b — Built-in fallback for the Fast DDS HelloWorld example.
        // Some subscribers announce the topic but do not expose complete
        // TypeLookup/TypeObject information. In that case we still enable
        // publishing using the canonical HelloWorld IDL.
        if (!publisherReady && topicName == "HelloWorldTopic")
        {
            const QString helloWorldIDL =
                "struct HelloWorld {\n"
                "  unsigned long index;\n"
                "  string message;\n"
                "};\n";

            qWarning() << "[Controller] TypeLookup unavailable for HelloWorldTopic."
                       << "Using built-in HelloWorld IDL fallback.";

            // Keep model/editor aligned with the actual type used to publish.
            topicIDLModel_->setTextData(helloWorldIDL);
            publisherReady = publisher->initializeFromIDLText(helloWorldIDL);
            qDebug() << "[Controller] HelloWorld fallback initializeFromIDLText returned:"
                     << publisherReady;
        }

        // Step 4 — Post result back to the Qt main thread.
        // Using QueuedConnection ensures m_publisherMap is only written from
        // the main thread, making subsequent reads in publishOneSample safe.
        if (publisherReady)
        {
            QMetaObject::invokeMethod(this,
                [this, key, publisher, subscriber, topicName]()
                {
                    qDebug() << "[Controller] ✓ Publisher ready — stored in map";
                    m_pendingDiscovery_.remove(key);
                    m_publisherMap[key] = publisher;
                    if (m_discoverySubscriberMap_.contains(key) &&
                        m_discoverySubscriberMap_[key] != subscriber)
                    {
                        delete m_discoverySubscriberMap_[key];
                    }
                    m_discoverySubscriberMap_[key] = subscriber;
                    if (!topicIDLModel_->textData().trimmed().isEmpty() &&
                        topicIDLModel_->topicName() == topicName)
                    {
                        m_topicIdlCache_[key] = topicIDLModel_->textData();
                    }
                    emit publisherTypeDiscovered(topicName);
                },
                Qt::QueuedConnection);
        }
        else
        {
            QString reason = idlText.trimmed().isEmpty()
                ? "No active DDS participant found for topic '" + topicName + "'.\n\n"
                "Ensure at least one publisher or subscriber is running\n"
                "on domain " + QString::number(key.first) + " for this topic."
                : "DDS entity initialization failed.\n\n"
                "IDL structure was discovered but DataWriter could not be created.\n"
                "Check QoS compatibility and domain ID.";

            QMetaObject::invokeMethod(this,
                [this, key, publisher, subscriber, topicName, reason]()
                {
                    qCritical() << "[Controller] ✗ Publisher NOT ready:" << reason;
                    m_pendingDiscovery_.remove(key);
                    if (m_discoverySubscriberMap_.contains(key) &&
                        m_discoverySubscriberMap_[key] == subscriber)
                    {
                        m_discoverySubscriberMap_.remove(key);
                    }
                    delete subscriber;
                    delete publisher;
                    emit publisherDiscoveryFailed(topicName, reason);
                },
                Qt::QueuedConnection);
        }

        qDebug() << "[Controller] Discovery thread exiting";
    });

    discovery_thread.detach();
    qDebug() << "[Controller] Discovery thread detached";
}

void Controller::destroyPublicationManager(PublicationManager *manager)
{
    qDebug() << "[Controller] Destroying PublicationManager";

    if (manager && m_publicationManagers.contains(manager))
    {
        m_publicationManagers.removeOne(manager);
        manager->deleteLater();
        emit publicationClosed();
    }
}

bool Controller::isTopicSubscribed(int domainId, const QString &topicName) const
{
    qDebug() << "[Controller::isTopicSubscribed] Checking subscription status";
    qDebug() << "[Controller::isTopicSubscribed] Domain ID:" << domainId;
    qDebug() << "[Controller::isTopicSubscribed] Topic Name:" << topicName;

    // Create the key for lookup
    QPair<int, QString> key = qMakePair(domainId, topicName);

    // Check if this topic is in our active subscriptions set
    bool subscribed = activeSubscriptions_.contains(key);

    qDebug() << "[Controller::isTopicSubscribed] Result: Topic IS"
             << (subscribed ? "SUBSCRIBED" : "NOT SUBSCRIBED");

    return subscribed;
}

void Controller::unsubscribeFromTopic(int domainId, const QString &topicName)
{
    qDebug() << "[Controller::unsubscribeFromTopic] ========================================";
    qDebug() << "[Controller::unsubscribeFromTopic] UNSUBSCRIBE REQUEST";
    qDebug() << "[Controller::unsubscribeFromTopic] Domain:" << domainId << "Topic:" << topicName;
    qDebug() << "[Controller::unsubscribeFromTopic] ========================================";

    QPair<int, QString> key = qMakePair(domainId, topicName);

    if (!activeSubscriptions_.contains(key))
    {
        qWarning() << "[Controller::unsubscribeFromTopic] ⚠ NOT SUBSCRIBED!";
        return;
    }

    // ========== GET THE SUBSCRIBER OBJECT ==========
    HelloWorldSubscriber *subscriber = subscriberMap_.value(key, nullptr);

    if (!subscriber)
    {
        qCritical() << "[Controller::unsubscribeFromTopic] ✗ ERROR: Subscriber NOT in map!";
        activeSubscriptions_.remove(key);
        return;
    }

    qDebug() << "[Controller::unsubscribeFromTopic] ✓ Subscriber found at:" << subscriber;

    // ========== STEP 1: Signal thread to stop ==========
    qDebug() << "[Controller::unsubscribeFromTopic] Calling stop() on subscriber...";
    subscriber->stop();
    qDebug() << "[Controller::unsubscribeFromTopic] ✓ stop() called";

    // ========== STEP 2: Wait for thread to exit ==========
    qDebug() << "[Controller::unsubscribeFromTopic] Waiting 1000ms for thread to exit...";
    QThread::msleep(1000); // Give thread time to exit run() loop

    // ========== STEP 3: Delete subscriber (triggers DDS cleanup in destructor) ==========
    qDebug() << "[Controller::unsubscribeFromTopic] Deleting subscriber object...";
    qDebug() << "[Controller::unsubscribeFromTopic] This will cleanup:";
    qDebug() << "[Controller::unsubscribeFromTopic]   - DataReaders";
    qDebug() << "[Controller::unsubscribeFromTopic]   - Topics";
    qDebug() << "[Controller::unsubscribeFromTopic]   - Subscriber";
    qDebug() << "[Controller::unsubscribeFromTopic]   - DomainParticipant";

    delete subscriber;

    qDebug() << "[Controller::unsubscribeFromTopic] ✓ Subscriber deleted";

    // ========== STEP 4: Remove from tracking ==========
    subscriberMap_.remove(key);
    activeSubscriptions_.remove(key);
    subscriberThreads_.remove(key);

    qDebug() << "[Controller::unsubscribeFromTopic] ✓ UNSUBSCRIBE COMPLETED";
    qDebug() << "[Controller::unsubscribeFromTopic] Active subscriptions remaining:"
             << activeSubscriptions_.size();
    qDebug() << "[Controller::unsubscribeFromTopic] ========================================";
}

bool Controller::publishOneSample(const QString &topicName, int domainId,
                                  const QVariantMap &sampleData)
{
    // publishOneSample() must be a pure lookup + write — no blocking,
    // no participant creation, no type discovery.  All of that is done
    // once, ahead of time, in startPublisherWithDiscovery().

    QPair<int, QString> key = qMakePair(domainId, topicName);
    HelloWorldPublisher* publisher = m_publisherMap.value(key, nullptr);

    if (!publisher || !publisher->isReady())
    {
        qWarning() << "[Controller::publishOneSample] Publisher missing/unready for"
                   << topicName << "domain" << domainId
                   << "— attempting lazy recovery from discovered IDL";

        if (!topicIDLModel_)
        {
            qCritical() << "[Controller::publishOneSample] ✗ topicIDLModel_ is NULL";
            return false;
        }

        QString recoveryIDL;
        if (topicIDLModel_->topicName() == topicName && !topicIDLModel_->textData().trimmed().isEmpty())
        {
            recoveryIDL = topicIDLModel_->textData();
        }
        else if (m_topicIdlCache_.contains(key))
        {
            recoveryIDL = m_topicIdlCache_.value(key);
            qDebug() << "[Controller::publishOneSample] Using cached IDL for" << topicName;
        }
        if (recoveryIDL.trimmed().isEmpty() && topicName == "HelloWorldTopic")
        {
            recoveryIDL =
                "struct HelloWorld {\n"
                "  unsigned long index;\n"
                "  string message;\n"
                "};\n";
            topicIDLModel_->setTextData(recoveryIDL);
            m_topicIdlCache_[key] = recoveryIDL;
            qWarning() << "[Controller::publishOneSample] Using HelloWorld built-in IDL"
                       << "for lazy publisher recovery";
        }

        if (recoveryIDL.trimmed().isEmpty())
        {
            qCritical() << "[Controller::publishOneSample] ✗ No discovered IDL text available for"
                        << topicName << "domain" << domainId;
            return false;
        }

        if (publisher)
        {
            delete publisher;
            m_publisherMap.remove(key);
            publisher = nullptr;
        }

        HelloWorldPublisher* recovered = new HelloWorldPublisher();
        if (!recovered->init(topicName.toStdString(), domainId, topicIDLModel_))
        {
            qCritical() << "[Controller::publishOneSample] ✗ Lazy recovery init() failed";
            delete recovered;
            return false;
        }

        bool recoveredReady = recovered->initializeFromIDLText(recoveryIDL);
        if (!recoveredReady)
        {
            qCritical() << "[Controller::publishOneSample] ✗ Lazy recovery initializeFromIDLText() failed";
            delete recovered;
            return false;
        }

        m_publisherMap[key] = recovered;
        m_topicIdlCache_[key] = recoveryIDL;
        publisher = recovered;
        qDebug() << "[Controller::publishOneSample] ✓ Lazy recovery succeeded, publisher ready";
    }

    qDebug() << "[Controller::publishOneSample] Writing sample to"
            << topicName << "domain" << domainId
            << "fields=" << sampleData.size();

    bool success = publisher->writeSample(sampleData);

    qDebug() << "[Controller::publishOneSample]" << (success ? "✓ SUCCESS" : "✗ FAILED");
    return success;
}

// ========================================
// DESTRUCTOR: Clean up publishers
// ========================================
Controller::~Controller()
{
    qDebug() << "[Controller::~Controller] ========================================";
    qDebug() << "[Controller::~Controller] Destructor called, cleaning up publishers...";
    qDebug() << "[Controller::~Controller] Total publishers in map:" << m_publisherMap.size();

    // Delete all publishers in the map
    for (auto it = m_publisherMap.begin(); it != m_publisherMap.end(); ++it)
    {
        qDebug() << "[Controller::~Controller] Deleting publisher for:"
                 << "Domain" << it.key().first
                 << "Topic" << it.key().second;

        if (it.value())
        {
            delete it.value();
            qDebug() << "[Controller::~Controller] ✓ Publisher deleted";
        }
        else
        {
            qWarning() << "[Controller::~Controller] ⚠ Publisher was NULL!";
        }
    }

    m_publisherMap.clear();

    for (auto it = m_discoverySubscriberMap_.begin(); it != m_discoverySubscriberMap_.end(); ++it)
    {
        if (it.value())
        {
            delete it.value();
        }
    }
    m_discoverySubscriberMap_.clear();

    qDebug() << "[Controller::~Controller] ✓ All publishers cleaned up";
    qDebug() << "[Controller::~Controller] ========================================";
}

QObject* Controller::get_match_analyses_model()
{
    if (!match_analyses_model_ && engine_) {
        // Create the Match Analyses Model lazily
        backend::SyncBackendConnection* backend = engine_->get_backend_connection();
        if (backend) {
            match_analyses_model_ = new MatchAnalysesModel(backend, this);
            emit matchAnalysesModelChanged();
            qDebug() << "[Controller] Match Analyses Model created";
        }
    }
    return match_analyses_model_;
}
