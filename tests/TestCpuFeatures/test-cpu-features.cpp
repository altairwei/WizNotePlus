#include <QtTest/QtTest>
#include <QDebug>
#include <QString>
#include <immintrin.h>
#include "quazip/quazip.h"

class TestCpuFeatures: public QObject
{
    Q_OBJECT

private slots:
    void testAVX();
    void testAVX2();
    void testQuaZip();

};

void TestCpuFeatures::testAVX()
{
    bool isAVXEnable = false;

#ifdef __AVX__
    /* Initialize the two argument vectors */
    __m256 evens = _mm256_set_ps(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0);
    __m256 odds = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0);

    /* Compute the difference between the two vectors */
    __m256 result = _mm256_sub_ps(evens, odds);

    /* Display the elements of the result vector */
    float* f = (float*)&result;
    qDebug() << f[0] << f[1] << f[2] << f[3] << f[4] << f[5] << f[6] << f[7];
    isAVXEnable = true;
#else
    isAVXEnable = false;
#endif

    QVERIFY(isAVXEnable);

}

void TestCpuFeatures::testAVX2()
{
    bool isAVX2Enable = false;

#ifdef __AVX2__
    int int_array[8] = {100, 200, 300, 400, 500, 600, 700, 800};

    /* Initialize the mask vector */
    __m256i mask = _mm256_setr_epi32(-20, -72, -48, -9, -100, 3, 5, 8);

    /* Selectively load data into the vector */
    __m256i result = _mm256_maskload_epi32(int_array, mask);

    /* Display the elements of the result vector */
    int* res = (int*)&result;

    qDebug() << res[0] << res[1] << res[2] << res[3] << res[4] << res[5] << res[6] << res[7];

    isAVX2Enable = true;
#else
    isAVX2Enable = false;
#endif

    QVERIFY(isAVX2Enable);
}

void TestCpuFeatures::testQuaZip()
{
    QuaZip zip;
    qDebug() << "Hello world, QuaZIP is opened: " << zip.isOpen();
}

QTEST_MAIN(TestCpuFeatures)
#include "test-cpu-features.moc"
