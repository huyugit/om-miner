#include "cStringType.h"
#include <stdarg.h>


cStringType::~cStringType()
{
    Clear();
}
/*
cStringType::cStringType(cStringType&& srcStr)
{
    TextBuffer=srcStr.TextBuffer;
    TextBufferSize=srcStr.TextBufferSize;

    srcStr.TextBuffer=NULL;
    srcStr.TextBufferSize=0;
}
void cStringType::operator = (cStringType&& srcStr)
{
    Clear();
    TextBuffer=srcStr.TextBuffer;
    TextBufferSize=srcStr.TextBufferSize;

    srcStr.TextBuffer=NULL;
    srcStr.TextBufferSize=0;
}
*/
cStringType::cStringType():TextBuffer(NULL),TextBufferSize(0)
{

}
cStringType::cStringType(size_t NewSize):TextBuffer(NULL),TextBufferSize(0)
{
    PrepareSize(NewSize);
}
bool cStringType::EatEOL()
{
    if(!TextBuffer)
        return false;

    size_t len=GetLength();
    if(len==0)
        return false;
    if(TextBuffer[len-1]=='\n')
    {
        PrepareSize();
        TextBuffer[len-1]='\0';
        return true;
    }else
        return false;
    return false;
}
char* cStringType::EatSpaces(char *Dest)
{
    size_t i;
    char* Begin=NULL;
    char* EndBegin=NULL;
    for(i=0;Dest[i]!='\0';i++)
    {
        if(!Begin && Dest[i]!=' ' && Dest[i]!='\t')
            Begin=&Dest[i];
        if(Dest[i]==' ' || Dest[i]=='\t')
        {
            if(!EndBegin)
                EndBegin=&Dest[i];
        }else
            EndBegin=NULL;
    }
    if(EndBegin)
        EndBegin[0]='\0';
    if(!Begin)
    {
        Dest[0]='\0';
        return Dest;
    }else
        return Begin;
}
const char* cStringType::EatBeginSpaces(const char *Dest)
{
    size_t i;
    const char* Begin=NULL;
//    const char* EndBegin=NULL;
    for(i=0;Dest[i]!='\0';i++)
    {
        if(Dest[i]!=' ' && Dest[i]!='\t')
        {
            Begin=&Dest[i];
            break;
        }
    }
    if(!Begin)
    {
        return &Dest[i];
    }else
        return Begin;
}
size_t cStringType::strcopy(char *Dest,const char *Src,size_t Len)
{
    size_t i;
    for (i = 0 ; i < Len && Src[i] != '\0' ; i++)
        Dest[i] = Src[i];
    if(i<Len)
    {
        Dest[i] = '\0';
        return i;
    }else
    {
        if(Len==0)
            return 0;
        Dest[Len-1] = '\0';
        return Len-1;
    }
}
void cStringType::OptimizeMemory()
{
    if(TextBufferSize==0)
        return;
    size_t Len=strlen(TextBuffer);
    Len=CalculateBufferSize(Len);
    if(TextBufferSize!=Len)
    {
        TextBufferSize=Len;
        if(TextBuffer)
        {
            TextBuffer=(char*)realloc(TextBuffer,TextBufferSize*sizeof(TextBuffer[0]));
        }else
            TextBuffer=(char*)malloc(TextBufferSize*sizeof(TextBuffer[0]));
    }
}
const char *cStringType::AssignStatic(const char *Text)
{
    if(TextBufferSize)
        free(TextBuffer);
    TextBuffer=(char*)Text;
    TextBufferSize=0;
    return TextBuffer;
}
const char *cStringType::operator =(const char *Text)
{
    size_t L=strlen(Text);
    PrepareSize(L);
    strncpy(TextBuffer,Text,TextBufferSize);
    TextBuffer[TextBufferSize-1]=0;
    return TextBuffer;
}
const char *cStringType::operator =(char Text)
{
    char Buff[2]={Text,'\0'};
    return (*this)=Buff;
}
const char *cStringType::operator +=(char Text)
{
    char Buff[2]={Text,'\0'};
    return (*this)+=Buff;
}
const char *cStringType::operator +=(const char *Text)
{
    size_t L=strlen(Text)+strlen(TextBuffer?TextBuffer:"");
    PrepareSize(L);
    strncat(TextBuffer,Text,TextBufferSize);
    TextBuffer[TextBufferSize-1]=0;
    return TextBuffer;
}
const char *cStringType::operator +=(uint32_t Val)
{
    char Buff[21];
    snprintf(Buff,20,"%u",Val);
    return (*this)+=Buff;
}
const char *cStringType::operator +=(uint64_t Val)
{
    char Buff[81];
    snprintf(Buff,80,"%lu",static_cast<long unsigned int>(Val));
    return (*this)+=Buff;
}
const char *cStringType::operator +=(int32_t Val)
{
    char Buff[21];
    snprintf(Buff,20,"%d",Val);
    return (*this)+=Buff;
}
const char *cStringType::operator =(uint32_t Val)
{
    char Buff[21];
    snprintf(Buff,20,"%u",Val);
    return (*this)=Buff;
}
const char *cStringType::operator =(uint64_t Val)
{
    char Buff[81];
    snprintf(Buff,80,"%lu",static_cast<long unsigned int>(Val));
    return (*this)=Buff;
}
const char *cStringType::operator =(int32_t Val)
{
    char Buff[21];
    snprintf(Buff,20,"%d",Val);
    return (*this)=Buff;
}
const char* cStringType::add_printf(const char *Str,...)
{
    PrepareSize(TextBufferSize+31);
    va_list vl;
	va_start(vl,Str);//инициализируем работу с переменным количеством параметров функции, после переменной FormatString
	size_t len=GetLength();
    int res=vsnprintf(&TextBuffer[len],TextBufferSize-len,Str,vl);
    va_end(vl);//закончим работу с переменным количеством параметров функции
    if(res>=(int)(TextBufferSize-len))
    {
        PrepareSize(len+res);
        va_start(vl,Str);//инициализируем работу с переменным количеством параметров функции, после переменной FormatString
        res=vsnprintf(&TextBuffer[len],TextBufferSize-len,Str,vl);
        va_end(vl);//закончим работу с переменным количеством параметров функции
        TextBuffer[len+res]=0;
    }
    return TextBuffer;
}
const char* cStringType::printf(const char *Str,...)
{
    PrepareSize(31);
    va_list vl;
	va_start(vl,Str);//инициализируем работу с переменным количеством параметров функции, после переменной FormatString
    int res=vsnprintf(TextBuffer,TextBufferSize,Str,vl);
    va_end(vl);//закончим работу с переменным количеством параметров функции
    if(res>=(int)TextBufferSize)
    {
        PrepareSize(res);
        va_start(vl,Str);//инициализируем работу с переменным количеством параметров функции, после переменной FormatString
        res=vsnprintf(TextBuffer,TextBufferSize,Str,vl);
        va_end(vl);//закончим работу с переменным количеством параметров функции
        TextBuffer[res]=0;
    }
    return TextBuffer;
}
void cStringType::PrepareSize(size_t S)
{
    if(S==0)//need same size like now
    {
        S=GetLength();
    }
    S++;
    if(TextBufferSize<S)
    {
        char *SaveBuffer=NULL;
        if(TextBufferSize==0)//указатель на константную строку
        {
            SaveBuffer=TextBuffer;
            TextBuffer=NULL;
            if(SaveBuffer)
            {
                size_t l=strlen(SaveBuffer)+1;
                if(l>S)
                    S=l;
            }
        }
        TextBufferSize=CalculateBufferSize(S);
        if(TextBuffer)
        {
            TextBuffer=(char*)realloc(TextBuffer,TextBufferSize*sizeof(TextBuffer[0]));
        }else
        {
            //char *SaveBuffer=TextBuffer;
            TextBuffer=(char*)malloc(TextBufferSize*sizeof(TextBuffer[0]));
            TextBuffer[0]=0;
            if(SaveBuffer)
                strncpy(TextBuffer,SaveBuffer,TextBufferSize);
            TextBuffer[TextBufferSize-1]=0;
        }
    }
}
cStringType::cStringType(const char *Text,bool Copy):TextBuffer(NULL),TextBufferSize(0)
{
    if(Copy)
    {
        TextBufferSize=CalculateBufferSize(strlen(Text));
        TextBuffer=(char*)malloc(TextBufferSize*(sizeof(TextBuffer[0])));
        strncpy(TextBuffer,Text,TextBufferSize);
        TextBuffer[TextBufferSize-1]=0;
    }else
    {
        TextBuffer=(char*)Text;
    }
}
cStringType::cStringType(const cStringType &Obj,bool Copy)
{
    const char *Text=Obj;
    if(Obj.TextBufferSize)//если там динамический буфер то надо копировать ибо там буфер может сменится
        Copy=true;
    if(Copy)
    {
        TextBufferSize=CalculateBufferSize(strlen(Text));
        TextBuffer=(char*)malloc(TextBufferSize*(sizeof(TextBuffer[0])));
        strncpy(TextBuffer,Text,TextBufferSize);
        TextBuffer[TextBufferSize-1]=0;
    }else
    {
        TextBuffer=(char*)Text;
    }
}
