#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h>  
#include<sys/stat.h>   
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>
#include<math.h>
#include <sys/time.h>  
#include <time.h>  
#define SERIAL_PORT "/dev/ttyUSB0" //串口地址
// 测试时使用的文件  
#define SRC_FILE_NAME     (const char *)"GPS.txt"  
// 文件头预留64个字节  
#define FILE_HEADER_LEN   64  

#define uchar unsigned char
#define uint  unsigned int
// timer 
int gettimeofday(struct timeval *tv, struct timezone *tz); 
typedef struct{
    int year;  
    int month; 
    int  day;
    int hour;
    int minute;
    int second;
}DATE_TIME;

typedef struct{
    double  latitude;  //经度
    double  longitude; //纬度
    int     latitude_Degree;    //度
    int        latitude_Cent;        //分
    int       latitude_Second;    //秒
    int     longitude_Degree;    //度
    int        longitude_Cent;        //分
    int       longitude_Second;   //秒
    float     speed;      //速度
    float     direction;  //航向
    float     height_ground;    //水平面高度
    float     height_sea;       //海拔高度
    uchar     NS;
    uchar     EW;
    DATE_TIME D;
    uchar status;          //接收状态
    int GPS_Num;               //使用卫星个数
}GPS_INFO;


GPS_INFO gps_info;//存储GPS信息变量

static uchar GetComma(uchar num,char* str);
static double Get_Double_Number(char *s);
static float Get_Float_Number(char *s);
static void UTC2BTC(DATE_TIME *GPS);

int GPS_RMC_Parse(char *line,GPS_INFO *GPS);
int GPS_GGA_Parse(char *line,GPS_INFO *GPS);

void Int_To_Str(int x,char *Str);

/*==============================================================================
函 数 名 : GetFile
功    能 : 获取文件句柄
算法实现 : 无
参    数 : [in] const char *pFileName - 文件名
返 回 值 : 成功-得到的文件句柄，失败-NULL
日    期 : 2011/02/11
作    者 : jernymy
==============================================================================*/
static FILE *GetFile(const char *pFileName)
{
    // 使用rt+的方式打开文件
    FILE *fpSrc = fopen(pFileName, "rt+");
    
    if (NULL != fpSrc)
    {
        // 文件存在
        return fpSrc;
    }
    printf("open %s \"rt\" fail, may be create first!/n", pFileName);
    // 创建文件
    fpSrc = fopen(pFileName, "wt");
    if (NULL != fpSrc)
    {
        // 文件创建成功
        printf("create %s \"wt\" succ, pointer:%p!/n", pFileName, fpSrc);
    }
    else
    {
        // 文件创建失败
        printf("create %s \"wt\" fail, pointer:%p!/n", pFileName, fpSrc);
    }
    return fpSrc;
}

/*==============================================================================
函 数 名 : WriteFile
功    能 : 写文件操作
算法实现 : 无
参    数 : [in] const char *pFileName - 文件名
           [in] const char *pchStr    - 写入的字符串buffer
返 回 值 : 成功-0，失败--1
日    期 : 2011/02/11
作    者 : jernymy
==============================================================================*/
static int WriteFile(const char *pFileName, const char *pchStr)
{
    FILE *fpSrc = NULL;
    int  nFileLen;
    if (NULL == pFileName)
    {
        printf("pFileName is NULL, exit!/n");
        return -1;
    }
    if (NULL == pchStr)
    {
        printf("pchStr is NULL, exit!/n");
        return -1;
    }
    fpSrc = GetFile(pFileName);
    if (NULL == fpSrc)
    {
        printf("get file fail! exit/n");
        return -1;
    }
    
    // 得到文件大小-文件长度
    fseek(fpSrc, 0L, SEEK_END);
    nFileLen = ftell(fpSrc);
    // 写文件头后面部分
    if (0 == nFileLen)
    {
        nFileLen = FILE_HEADER_LEN;
    }
    fseek(fpSrc, nFileLen, SEEK_SET);
    if (FILE_HEADER_LEN == nFileLen)
    {
        fprintf(fpSrc, "\n");//用于写文件头部分
    }
    fprintf(fpSrc, "%s\n", pchStr);
    // 写文件头部分
    fseek(fpSrc, 0L, SEEK_END);
    nFileLen = ftell(fpSrc);
    fseek(fpSrc, 0L, SEEK_SET);
    fprintf(fpSrc, "#FavorGPS#File size:%09d", nFileLen);
    // 关闭文件
    if (NULL != fpSrc)
    {
        fclose(fpSrc);
        fpSrc = NULL;
    }
    return 0;
}

void show_gps(GPS_INFO *GPS)
{
    printf("STATUS   : %c\n",GPS->status);
    printf("DATE     : %4d-%02d-%02d \n",GPS->D.year,GPS->D.month,GPS->D.day);
    printf("TIME     :  %02d:%02d:%02d \n",GPS->D.hour,GPS->D.minute,GPS->D.second);
    printf("Latitude : %10.8f %c\n",GPS->latitude,GPS->NS);    
    printf("Longitude: %10.8f %c\n",GPS->longitude,GPS->EW);    
    printf("high     : %10.4f \n",GPS->height_sea);    
    printf("Speed   : %10.4f Km/h\n",GPS->speed);
    printf("GPS_Num   : %2d\n",GPS->GPS_Num);
    //    
    if(GPS->status=='A')
    {
//    char pchStr[1024];
//    sprintf(pchStr,"%4d-%02d-%02d %02d:%02d:%02d|%10.8f %c|%10.8f %c|%10.4f",GPS->D.year,GPS->D.month,GPS->D.day,GPS->D.hour,GPS->D.minute,GPS->D.second,GPS->latitude,GPS->NS,GPS->longitude,GPS->EW,GPS->height_sea);
    
//    WriteFile(SRC_FILE_NAME, pchStr);    //写GPS信息到文件    
    }
}
//====================================================================//
// 语法格式：int GPS_RMC_Parse(char *line, GPS_INFO *GPS)  
// 实现功能：把gps模块的GPRMC信息解析为可识别的数据
// 参    数：存放原始信息字符数组、存储可识别数据的结构体
// 返 回 值：
//             1: 解析GPRMC完毕
//           0: 没有进行解析，或数据无效
//====================================================================//
int GPS_RMC_Parse(char *line,GPS_INFO *GPS)
{
    uchar ch, status, tmp;
    float lati_cent_tmp, lati_second_tmp;
    float long_cent_tmp, long_second_tmp;
    float speed_tmp;
    char *buf = line;
    ch = buf[5];
    status = buf[GetComma(2, buf)];
    GPS->status =status;
    if (ch == 'C')  //如果第五个字符是C，($GPRMC)
    {
        if (status == 'A')  //如果数据有效，则分析
        {
            GPS -> NS       = buf[GetComma(4, buf)];
            GPS -> EW       = buf[GetComma(6, buf)];

            GPS->latitude   = Get_Double_Number(&buf[GetComma(3, buf)])*0.01;
            GPS->longitude  = Get_Double_Number(&buf[GetComma( 5, buf)])*0.01;

               GPS->latitude_Degree  = (int)GPS->latitude / 100;       //分离纬度
            lati_cent_tmp         = (GPS->latitude - GPS->latitude_Degree * 100);
            GPS->latitude_Cent    = (int)lati_cent_tmp;
            lati_second_tmp       = (lati_cent_tmp - GPS->latitude_Cent) * 60;
            GPS->latitude_Second  = (int)lati_second_tmp;

            GPS->longitude_Degree = (int)GPS->longitude / 100;    //分离经度
            long_cent_tmp         = (GPS->longitude - GPS->longitude_Degree * 100);
            GPS->longitude_Cent   = (int)long_cent_tmp;    
            long_second_tmp       = (long_cent_tmp - GPS->longitude_Cent) * 60;
            GPS->longitude_Second = (int)long_second_tmp;

            speed_tmp      = Get_Float_Number(&buf[GetComma(7, buf)]);    //速度(单位：海里/时)
            GPS->speed     = speed_tmp * 1.85;                           //1海里=1.85公里
            GPS->direction = Get_Float_Number(&buf[GetComma(8, buf)]); //角度            

            GPS->D.hour    = (buf[7] - '0') * 10 + (buf[8] - '0');        //时间
            GPS->D.minute  = (buf[9] - '0') * 10 + (buf[10] - '0');
            GPS->D.second  = (buf[11] - '0') * 10 + (buf[12] - '0');
            tmp = GetComma(9, buf);
            GPS->D.day     = (buf[tmp + 0] - '0') * 10 + (buf[tmp + 1] - '0'); //日期
            GPS->D.month   = (buf[tmp + 2] - '0') * 10 + (buf[tmp + 3] - '0');
            GPS->D.year    = (buf[tmp + 4] - '0') * 10 + (buf[tmp + 5] - '0')+2000;
            
            UTC2BTC(&GPS->D);
            
            return 1;
        }        
    }
    
    return 0;
}

//====================================================================//
// 语法格式：int GPS_GGA_Parse(char *line, GPS_INFO *GPS)  
// 实现功能：把gps模块的GPGGA信息解析为可识别的数据
// 参    数：存放原始信息字符数组、存储可识别数据的结构体
// 返 回 值：
//             1: 解析GPGGA完毕
//           0: 没有进行解析，或数据无效
//====================================================================//
int GPS_GGA_Parse(char *line,GPS_INFO *GPS)
{
    uchar ch, status;
    char *buf = line;
    ch = buf[4];
    status = buf[GetComma(2, buf)];
    if (ch == 'G')  //$GPGGA
    {
        if (status != ',')
        {
            GPS->height_sea = Get_Float_Number(&buf[GetComma(9, buf)]);
            GPS->height_ground = Get_Float_Number(&buf[GetComma(11, buf)]);
            GPS->GPS_Num=(int)Get_Double_Number(&buf[GetComma(7, buf)]);
            return 1;
        }
    }
    
    return 0;
}

//====================================================================//
// 语法格式: static float Str_To_Float(char *buf)
// 实现功能： 把一个字符串转化成浮点数
// 参    数：字符串
// 返 回 值：转化后单精度值
//====================================================================//
static float Str_To_Float(char *buf)
{
    float rev = 0;
    float dat;
    int integer = 1;
    char *str = buf;
    int i;
    while(*str != '\0')
    {
        switch(*str)
        {
            case '0':
                dat = 0;
                break;
            case '1':
                dat = 1;
                break;
            case '2':
                dat = 2;
                break;        
            case '3':
                dat = 3;
                break;
            case '4':
                dat = 4;
                break;
            case '5':
                dat = 5;
                break;
            case '6':
                dat = 6;
                break;
            case '7':
                dat = 7;
                break;
            case '8':
                dat = 8;
                break;
            case '9':
                dat = 9;
                break;
            case '.':
                dat = '.';
                break;
        }
        if(dat == '.')
        {
            integer = 0;
            i = 1;
            str ++;
            continue;
        }
        if( integer == 1 )
        {
            rev = rev * 10 + dat;
        }
        else
        {
            rev = rev + dat / (10 * i);
            i = i * 10 ;
        }
        str ++;
    }
    return rev;

}
                                                
//====================================================================//
// 语法格式: static float Get_Float_Number(char *s)
// 实现功能： 把给定字符串第一个逗号之前的字符转化成单精度型
// 参    数：字符串
// 返 回 值：转化后单精度值
//====================================================================//
static float Get_Float_Number(char *s)
{
    char buf[10];
    uchar i;
    float rev;
    i=GetComma(1, s);
    i = i - 1;
    strncpy(buf, s, i);
    buf[i] = 0;
    rev=Str_To_Float(buf);
    return rev;    
}

//====================================================================//
// 语法格式: static double Str_To_Double(char *buf)
// 实现功能： 把一个字符串转化成浮点数
// 参    数：字符串
// 返 回 值：转化后双精度值
//====================================================================//
static double Str_To_Double(char *buf)
{
    double rev = 0;
    double dat;
    int integer = 1;
    char *str = buf;
    int i;
    while(*str != '\0')
    {
        switch(*str)
        {
            case '0':
                dat = 0;
                break;
            case '1':
                dat = 1;
                break;
            case '2':
                dat = 2;
                break;        
            case '3':
                dat = 3;
                break;
            case '4':
                dat = 4;
                break;
            case '5':
                dat = 5;
                break;
            case '6':
                dat = 6;
                break;
            case '7':
                dat = 7;
                break;
            case '8':
                dat = 8;
                break;
            case '9':
                dat = 9;
                break;
            case '.':
                dat = '.';
                break;
        }
        if(dat == '.')
        {
            integer = 0;
            i = 1;
            str ++;
            continue;
        }
        if( integer == 1 )
        {
            rev = rev * 10 + dat;
        }
        else
        {
            rev = rev + dat / (10 * i);
            i = i * 10 ;
        }
        str ++;
    }
    return rev;
}
                                                
//====================================================================//
// 语法格式: static double Get_Double_Number(char *s)
// 实现功能：把给定字符串第一个逗号之前的字符转化成双精度型
// 参    数：字符串
// 返 回 值：转化后双精度值
//====================================================================//
static double Get_Double_Number(char *s)
{
    char buf[10];
    uchar i;
    double rev;
    i=GetComma(1, s);
    i = i - 1;
    strncpy(buf, s, i);
    buf[i] = 0;
    rev=Str_To_Double(buf);
    return rev;    
}

//====================================================================//
// 语法格式：static uchar GetComma(uchar num,char *str)
// 实现功能：计算字符串中各个逗号的位置
// 参    数：查找的逗号是第几个的个数，需要查找的字符串
// 返 回 值：0
//====================================================================//
static uchar GetComma(uchar num,char *str)
{
    uchar i,j = 0;
    int len=strlen(str);

    for(i = 0;i < len;i ++)
    {
        if(str[i] == ',')
            j++;
        if(j == num)
            return i + 1;    
    }

    return 0;    
}

//====================================================================//
// 语法格式：void UTC2BTC(DATE_TIME *GPS)
// 实现功能：转化时间为北京时区的时间
// 参    数：存放时间的结构体
// 返 回 值：无
//====================================================================//
static void UTC2BTC(DATE_TIME *GPS)
{
    GPS->second ++;  
    if(GPS->second > 59)
    {
        GPS->second = 0;
        GPS->minute ++;
        if(GPS->minute > 59)
        {
            GPS->minute = 0;
            GPS->hour ++;
        }
    }    

    GPS->hour = GPS->hour + 8;
    if(GPS->hour > 23)
    {
        GPS->hour -= 24;
        GPS->day += 1;
        if(GPS->month == 2 ||
                   GPS->month == 4 ||
                   GPS->month == 6 ||
                   GPS->month == 9 ||
                   GPS->month == 11 )
        {
            if(GPS->day > 30)
            {
                   GPS->day = 1;
                GPS->month++;
            }
        }
        else
        {
            if(GPS->day > 31)
            {    
                   GPS->day = 1;
                GPS->month ++;
            }
        }
        if(GPS->year % 4 == 0 )
        {
               if(GPS->day > 29 && GPS->month == 2)
            {        
                   GPS->day = 1;
                GPS->month ++;
            }
        }
        else
        {
               if(GPS->day > 28 &&GPS->month == 2)
            {
                   GPS->day = 1;
                GPS->month ++;
            }
        }
        if(GPS->month > 12)
        {
            GPS->month -= 12;
            GPS->year ++;
        }        
    }
}
//====================================================================//
//    语法格式：    Int_To_Str(int x,char *Str)
//    实现功能：    转化整型值为字符串形式
//    参数：        x: 转化的整数
//                Str:转化后的字符串
//    返回值：    无
//====================================================================//
void Int_To_Str(int x,char *Str)
{
    int t;
    char *Ptr,Buf[5];
    int i = 0;
    Ptr = Str;
    if(x < 10)        // 当整数小于10时,转化为"0x"的格式
    {
        *Ptr ++ = '0';
        *Ptr ++ = x+0x30;
    }
    else
    {
        while(x > 0)
        {
            t = x % 10;
            x = x / 10;
            Buf[i++] = t+0x30;    // 通过计算把数字转化成ASCII码形式
        }
        i -- ;
        for(;i >= 0;i --)         // 将得到的字符串倒序
        {
            *(Ptr++) = Buf[i];
        }
    }
    *Ptr = '\0';
}
 
 int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0) { 
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': 
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':  
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
        newtio.c_cflag &=  ~CSTOPB;
    else if ( nStop == 2 )
    newtio.c_cflag |=  CSTOPB;
    newtio.c_cc[VTIME]  = 0;//重要
    newtio.c_cc[VMIN] = 100;//返回的最小值  重要
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
//    printf("set done!\n\r");
    return 0;
}
 
 
int main(void)
{
   //timer
    struct timeval tv;
    struct tm *gmt, *area;
    int buff_num=0;//缓存中字符数为0 
    int fd1,nset1,nread;
    char buf[1024];
    char buff[100],read_buf[1024]; 
    char comName[30];
    char comPath[80]="/dev/";
    int  index=0;
/*
    printf("please input COM Name:");
    scanf("%s",comName);

    fd1 = open(strcat(comPath,comName), O_RDWR);//打开串口
    if (fd1 == -1){
        printf("\r\n Open COM Port Falid! \r\n");
        exit(1);
        }

    nset1 = set_opt(fd1,115200, 8, 'N', 1);//设置串口属性
    if (nset1 == -1){
        printf("\r\n Set COM Port Falid! \r\n");
        exit(1);
        }
*/
       int fd;
    struct termios opt;
    fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY); //默认为阻塞读方式
    if (fd == -1)
    {
        perror("open serial 0\n");
        return 0;
    }

    tcgetattr(fd, &opt);
    cfsetispeed(&opt, B115200);

    if (tcsetattr(fd, TCSANOW, &opt) != 0)
    {
        perror("tcsetattr error");
        return 0;
    }

    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |= CS8;
    opt.c_cflag &= ~CSTOPB;
    opt.c_cflag &= ~PARENB;
    opt.c_cflag &= ~INPCK;
    opt.c_cflag |= (CLOCAL | CREAD);

    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    opt.c_oflag &= ~OPOST;
    opt.c_oflag &= ~(ONLCR | OCRNL);

    opt.c_iflag &= ~(ICRNL | INLCR);
    opt.c_iflag &= ~(IXON | IXOFF | IXANY);

    opt.c_cc[VTIME] = 0;
    opt.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    printf("configure complete\n");

    //if (tcsetattr(fd, TCSANOW, &opt) != 0)
    //{
    //    perror("serial error");
    //    return 0;
    //}
    printf("start send and receive data\n");

    //FILE *fpWrite=fopen("./data.txt","rw+");  
    FILE *fpWrite=fopen("./data.txt","wb");  
    if(fpWrite==NULL)  
    {  
	    printf("file open error\n");
        return 0;  
    }  
      
    while(1)
    {
                printf( "gps read"); //输出所读数据
            nread = read(fd, read_buf, 1024);//读串口 
                printf( "gps read fin"); //输出所读数据
            if (nread > 0){ 
                printf( "GPS:\n %s\n", read_buf); //输出所读数据
                printf("\r\n++++++++++++++++++++++++++++++++++++++++++\r\n");
                    
                /* find "$GPGGA" from raw_buf */
                char *wellhandled_stringGPGGA;
                if((wellhandled_stringGPGGA = strstr(read_buf, "$GPGGA"))!=NULL)
                {
                    int i;
                    for (i=0; i<strlen(wellhandled_stringGPGGA); i++)
                    {
                        if (wellhandled_stringGPGGA[i] == '\n')
                        {
                            wellhandled_stringGPGGA[i] = '\0'; //replace ‘\n’ with null
                        }
                    }                 
                    printf("%s\n",wellhandled_stringGPGGA);
                    //解析GPGGA
     
                    GPS_GGA_Parse(wellhandled_stringGPGGA,&gps_info);        
                }
                
                /* find "$GPRMC" from raw_buf */
                char *wellhandled_stringGPRMC;
                if((wellhandled_stringGPRMC = strstr(read_buf, "$GPRMC"))!=NULL)
                {
                    int i;
                    for (i=0; i<strlen(wellhandled_stringGPRMC); i++)
                    {
                        if (wellhandled_stringGPRMC[i] == '\n')
                        {
                            wellhandled_stringGPRMC[i] = '\0'; //replace ‘\n’ with null
                        }
                    }        
                    printf("%s\n",wellhandled_stringGPRMC);
                    //解析GPRMC
                    GPS_RMC_Parse(wellhandled_stringGPRMC,&gps_info);
                }
                show_gps(&gps_info);
                char pchStr[1024];
                // show time
                gettimeofday(&tv,0);
            area = localtime(&(tv.tv_sec));//以本地时区显示时间
            sprintf(pchStr,"time: %s %u:%u Latitude : %10.8f Longitude: %10.8f", 
                        asctime(area),tv.tv_sec,tv.tv_usec,gps_info.latitude,gps_info.longitude);
 //             sprintf(pchStr,"Latitude : %10.8f Longitude: %10.8f",gps_info.latitude,gps_info.longitude);  
                WriteFile(SRC_FILE_NAME, pchStr);     //写GPS信息到文件
                index++;
                printf("index=%d\r\n",index); 
                printf("\r\n++++++++++++++++++++++++++++++++++++++++++\r\n");        
            }    
        sleep(1);//睡眠，等待数据多一点
    }
    close(fd);
    fclose(fpWrite);
    return 0;
}
