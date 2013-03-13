// http://www.atmarkit.co.jp/ait/articles/0708/17/news078_3.html
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

int main( int argc, char* argv[] ) {
    // ���٥�ȡ��ޥ͡�����ȥƥ��ȡ�����ȥ������������
    CPPUNIT_NS::TestResult controller;

    // �ƥ��ȷ�̼����ꥹ�ʤ򥳥�ȥ���˥����å�����
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener( &result );

    // ��.�פǿʹԾ�������Ϥ���ꥹ�ʤ򥢥��å�����
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener( &progress );

    // �ƥ��ȡ����ʡ��˥ƥ��ȷ���Ϳ�����ƥ��Ȥ���
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run( controller );

    // �ƥ��ȷ�̤�ɸ����Ϥ��Ǥ��Ф�
    CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
