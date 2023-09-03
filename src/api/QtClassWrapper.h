#ifndef QTCLASSWRAPPER_H
#define QTCLASSWRAPPER_H

#include <QObject>
#include <QProcess>

namespace QtWrapper {

class Process : public QProcess
{
    Q_OBJECT

public:
    explicit Process(QObject *parent = nullptr)
        : QProcess(parent) {};

    Q_INVOKABLE void SetProgram(const QString &program) { setProgram(program); }
    Q_INVOKABLE void SetArguments(const QStringList &args) { setArguments(args); }
    Q_INVOKABLE void Start() { start(); }
    Q_INVOKABLE void Start(const QString &program, const QStringList &arguments) { start(program, arguments); }
    Q_INVOKABLE bool WaitForStarted(int msecs = 30000) { return waitForStarted(msecs); }
    Q_INVOKABLE bool WaitForFinished(int msecs = 30000) { return waitForFinished(msecs); }
    Q_INVOKABLE bool WaitForReadyRead(int msecs = 30000) { return waitForReadyRead(msecs); }

    Q_INVOKABLE QProcess::ProcessState State() { return state(); }
    Q_INVOKABLE int ExitCode() { return exitCode(); }
    Q_INVOKABLE QProcess::ProcessError Error() { return error(); }
    Q_INVOKABLE QProcess::ExitStatus ExitStatus() { return exitStatus(); }

    Q_INVOKABLE qint64 Write(const QString &data) { return write(data.toUtf8()); }
    Q_INVOKABLE void CloseWriteChannel() { closeWriteChannel(); }

    Q_INVOKABLE QString ReadAllStandardOutput() { return readAllStandardOutput(); }
    Q_INVOKABLE QString ReadAllStandardError() { return readAllStandardError(); }

    Q_INVOKABLE void SetWorkingDirectory(const QString &dir) { setWorkingDirectory(dir); }
    Q_INVOKABLE void AddProcessEnvironment(const QString &key, const QString &value);

    Q_INVOKABLE QString WorkingDirectory() { return workingDirectory(); }
    Q_INVOKABLE void SetStandardInputFile(const QString &fileName) { setStandardInputFile(fileName); }
    Q_INVOKABLE void SetStandardOutputFile(const QString &fileName, bool append = false);
    Q_INVOKABLE void SetStandardErrorFile(const QString &fileName, bool append = false);
};

}



#endif // QTCLASSWRAPPER_H
