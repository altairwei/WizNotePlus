#include "QtClassWrapper.h"

namespace QtWrapper {

void Process::AddProcessEnvironment(const QString &key, const QString &value)
{
    QProcessEnvironment env = processEnvironment();
    env.insert(key, value);
    setProcessEnvironment(env);
}

void Process::SetStandardOutputFile(const QString &fileName, bool append)
{
    setStandardOutputFile(fileName, append ? Append : Truncate);
}

void Process::SetStandardErrorFile(const QString &fileName, bool append)
{
    setStandardErrorFile(fileName, append ? Append : Truncate);
}

}

