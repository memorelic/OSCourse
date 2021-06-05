#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QDebug>

#pragma pack(push)
#pragma pack(1)

struct RootEntry
{
    // 文件名(8)+后缀名(3)
    char DIR_Name[11];
    // 文件属性
    uchar DIR_Attr;
    // 空位置
    uchar reserve[10];
    // 最后一次写入时间
    ushort DIR_WrtTime;
    // 最后一次写入日期
    ushort DIR_WrtDate;
    // 文件开始的簇号
    ushort DIR_FstClus;
    // 文件大小
    uint DIR_FileSize;

};

struct Fat12Header
{
    // OEM字符串，必须8字符，不足填空格
    char BS_OEMName[8];
    // 每扇区字节数
    ushort BPB_BytsPerSec;
    // 每簇扇区数
    uchar BPB_SecPerClus;
    // Boot占用扇区数
    ushort BPB_RsvdSecCnt;
    // FAT表的记录数
    uchar BPB_NumFATs;
    // 最大根目录文件数
    ushort BPB_RootEntCnt;
    // 逻辑扇区总数
    ushort BPB_TotSec16;
    // 媒体描述符
    uchar BPB_Media;
    // 每个FAT占用扇区数
    ushort BPB_FATSz16;
    // 每磁道扇区数
    ushort BPB_SecPerTrk;
    // 磁头数
    ushort BPB_NumHeads;
    // 隐藏扇区数
    uint BPB_HiddSec;
    // 如果BPB_TotSec16 == 0, 则此处记录扇区总数
    uint BPB_TotSec32;
    // 中断13的驱动器号
    uchar BS_DrvNum;
    // 魏使用
    uchar BS_Reserved1;
    // 扩展引导标志
    uchar BS_BootSig;
    // 卷序列号
    uint BS_VolID;
    // 卷标，必须11字符，不足以空格补充
    char BS_VolLab[11];
    // 文件系统类型，不足以8字符补充
    char BS_FileSysType[8];
};

#pragma pack(pop)

void PrintHeader(Fat12Header& rf, QString p)
{
    QFile file(p);

    if( file.open(QIODevice::ReadOnly) )
    {
        QDataStream in(&file);

        file.seek(3);

        in.readRawData(reinterpret_cast<char*>(&rf), sizeof(rf));

        // 将最后一位赋值为0，当作char*字符串使用
        rf.BS_OEMName[7] = 0;
        rf.BS_VolLab[10] = 0;
        rf.BS_FileSysType[7] = 0;

        qDebug() << "BS_OEMName: " << rf.BS_OEMName;
        qDebug() << "BPB_BytsPerSec: " << hex << rf.BPB_BytsPerSec;
        qDebug() << "BPB_SecPerClus: " << hex << rf.BPB_SecPerClus;
        qDebug() << "BPB_RsvdSecCnt: " << hex << rf.BPB_RsvdSecCnt;
        qDebug() << "BPB_NumFATs: " << hex << rf.BPB_NumFATs;
        qDebug() << "BPB_RootEntCnt: " << hex << rf.BPB_RootEntCnt;
        qDebug() << "BPB_TotSec16: " << hex << rf.BPB_TotSec16;
        qDebug() << "BPB_Media: " << hex << rf.BPB_Media;
        qDebug() << "BPB_FATSz16: " << hex << rf.BPB_FATSz16;
        qDebug() << "BPB_SecPerTrk: " << hex << rf.BPB_SecPerTrk;
        qDebug() << "BPB_NumHeads: " << hex << rf.BPB_NumHeads;
        qDebug() << "BPB_HiddSec: " << hex << rf.BPB_HiddSec;
        qDebug() << "BPB_TotSec32: " << hex << rf.BPB_TotSec32;
        qDebug() << "BS_DrvNum: " << hex << rf.BS_DrvNum;
        qDebug() << "BS_Reserved1: " << hex << rf.BS_Reserved1;
        qDebug() << "BS_BootSig: " << hex << rf.BS_BootSig;
        qDebug() << "BS_VolID: " << hex << rf.BS_VolID;
        qDebug() << "BS_VolLab: " << rf.BS_VolLab;
        qDebug() << "BS_FileSysType: " << rf.BS_FileSysType;

        file.seek(510);

        uchar b510 = 0;
        uchar b511 = 0;

        in.readRawData(reinterpret_cast<char*>(&b510), sizeof(b510));
        in.readRawData(reinterpret_cast<char*>(&b511), sizeof(b511));

        qDebug() << "Byte 510: " << hex << b510;
        qDebug() << "Byte 511: " << hex << b511;
    }

    file.close();
}

RootEntry FindRootEntry(Fat12Header& rf, QString p, int i)
{
    RootEntry ret = { {0} };

    QFile file(p);

    if (file.open(QIODevice::ReadOnly) && (0 <= i) && (i < rf.BPB_RootEntCnt))
    {
        QDataStream in(&file);

        // 根目录起始扇区
        constexpr uint RootStartSecNum = 19;

        file.seek(RootStartSecNum * rf.BPB_BytsPerSec + i * sizeof(RootEntry));

        in.readRawData(reinterpret_cast<char*>(&ret), sizeof(ret));
    }

    file.close();

    return ret;
}

void PrintRootEntry(Fat12Header& rf, QString p)
{
    for (int i = 0; i < rf.BPB_RootEntCnt; i++)
    {
        RootEntry re = FindRootEntry(rf, p, i);

        if (re.DIR_Name[0] != '\0')
        {
            qDebug() << i << ":";
            qDebug() << "DIR_Name: " << re.DIR_Name;
            qDebug() << "DIR_Attr: " << re.DIR_Attr;
            qDebug() << "DIR_WrtDate: " << re.DIR_WrtDate;
            qDebug() << "DIR_WrtTime: " << re.DIR_WrtTime;
            qDebug() << "DIR_WrtDate: " << re.DIR_WrtDate;
            qDebug() << "DIR_FstClus: " << re.DIR_FstClus;
            qDebug() << "DIR_FileSize: " << re.DIR_FileSize;
        }
    }
}

RootEntry FindRootEntry(Fat12Header& rf, QString p, QString fn)
{
    RootEntry ret = { {0} };

    for (int i = 0; i < rf.BPB_RootEntCnt; i++)
    {
        RootEntry re = FindRootEntry(rf, p, i);

        if (re.DIR_Name[0] != '\0')
        {
            int d = fn.lastIndexOf(".");
            QString name = QString(re.DIR_Name).trimmed();

            if (d >= 0)
            {
                QString preDotString = fn.mid(0, d);
                QString backDotString = fn.mid(d + 1);

                if (name.startsWith(preDotString) && name.endsWith(backDotString))
                {
                    ret = re;
                    break;
                }
            }
            else
            {
                if (name == fn)
                {
                    ret = re;
                    break;
                }
            }
        }
    }
    return ret;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString img = "./data.img";
    Fat12Header f12;

    qDebug() << "Print Header: ";
    PrintHeader(f12, img);
    qDebug() << endl;

    qDebug() << "Print RootEntry: ";
    // PrintRootEntry(f12, img);
    RootEntry re = FindRootEntry(f12, img, "LOADER.BIN");

    qDebug() << "DIR_Name: " << re.DIR_Name;
    qDebug() << "DIR_Attr: " << re.DIR_Attr;
    qDebug() << "DIR_WrtDate: " << re.DIR_WrtDate;
    qDebug() << "DIR_WrtTime: " << re.DIR_WrtTime;
    qDebug() << "DIR_WrtDate: " << re.DIR_WrtDate;
    qDebug() << "DIR_FstClus: " << re.DIR_FstClus;
    qDebug() << "DIR_FileSize: " << re.DIR_FileSize;

    return a.exec();
}
