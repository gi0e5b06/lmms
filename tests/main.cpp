#include "QTestSuite.h"

#include <QtTest/QTest>

#include <QDebug>

#include "MemoryManager.h"
#include "Configuration.h"
#include "Engine.h"

int main(int argc, char* argv[])
{
	MM_INIT //MemoryManager::init();
        ConfigManager::init(argv[0]);
        lmms_default_configuration();

	new QCoreApplication(argc, argv);
	Engine::init(true);

	int numsuites = QTestSuite::suites().size();
	qDebug() << ">> Will run" << numsuites << "test suites";
	int failed = 0;
	for (QTestSuite*& suite : QTestSuite::suites())
	{
		failed += QTest::qExec(suite, argc, argv);
	}
	qDebug() << "<<" << failed << "out of"<<numsuites<<"test suites failed.";
	return failed;
}
