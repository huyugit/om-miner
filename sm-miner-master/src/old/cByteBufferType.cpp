/*
 * Contains cByteBufferType class definition.
 */

#include "cByteBufferType.h"

#include "base/BaseUtil.h"
#include "base/MiscUtil.h"


inline char GetHexChar(uint8_t v)
{
    if(v<10)
        return '0'+v;
    if(v<16)
        return 'a'+(v-10);
    return '?';
}

void DataToHex(const uint8_t *k,char *Str,uint32_t len,bool needend)
{
    uint32_t i;
    for(i=0;i<len;i++)
    {
        Str[i*2]=GetHexChar(k[i]>>4);
        Str[i*2+1]=GetHexChar(k[i]&0xF);
    }
    if(needend)
        Str[i*2]='\0';
}

inline cByteBufferType operator+(const char *str,const cByteBufferType& obj)
{
    cByteBufferType k(str);
    k+=obj;
    return k;
}

inline cByteBufferType operator+(const cByteBufferType& obj1,const cByteBufferType& obj2)
{
    cByteBufferType k(obj1);
    k+=obj2;
    return k;
}

void cByteBufferType::SwapDWORDEndians()
{
    if(Len%4)
    {
        printf("error length for SwapDWORDEndians\n");
        //for(uint32_t i=0,l=4-Len%4;i<l;i++)
        //    Buffer[Len++]=0;
        return;
    }
    uint32_t *v=(uint32_t*)Buffer;
    for(uint32_t i=0,l=Len/4;i<l;i++)
    {
        v[i]=util::swapEndian(v[i]);
    }
}

void cByteBufferType::swap_array()
{
    uint8_t k;
    uint32_t l=Len/2;
    for(uint32_t i=0;i<l;i++)
    {
        k=Buffer[i];
        Buffer[i]=Buffer[Len-i-1];
        Buffer[Len-i-1]=k;
    }
}

uint32_t cByteBufferType::HexToBuff(const char *Str,uint8_t *res,uint32_t len)
{
    uint32_t Pos=0;
    while(Str[0]!='\0' && Pos<len)
    {
        uint8_t Val=0;
        res[Pos]=0;
        if(Str[0]>='0' && Str[0]<='9')
        {
            Val=Str[0]-'0';
        }else if(Str[0]>='a' && Str[0]<='f')
        {
            Val=Str[0]-'a'+10;
        }else if(Str[0]>='A' && Str[0]<='F')
        {
            Val=Str[0]-'A'+10;
        }else
            return Pos;
        Str++;
        if(Str[0]=='\0')
        {
            res[Pos]=Val;
            return Pos;
        }
        Val<<=4;
        if(Str[0]>='0' && Str[0]<='9')
        {
            Val|=Str[0]-'0';
        }else if(Str[0]>='a' && Str[0]<='f')
        {
            Val|=Str[0]-'a'+10;
        }else if(Str[0]>='A' && Str[0]<='F')
        {
            Val|=Str[0]-'A'+10;
        }else
            return Pos;
        res[Pos]=Val;
        Str++;
        Pos++;
    }
    return Pos;
}

void cByteBufferType::PrepareBuffer(uint32_t S)
{
    if(S==0)//need same size like now
    {
        S=Len;
    }
    if(S==0)
    {
        FreeBuffer();
        return;
    }
    if(Size<S)
    {
        Size=CalculateBufferSize(S);
        if(Buffer)
        {
            Buffer=(uint8_t*)realloc(Buffer,Size*sizeof(Buffer[0]));
        }else
        {
            Buffer=(uint8_t*)malloc(Size*sizeof(Buffer[0]));
        }
    }
}

void cByteBufferType::dump()
{
    util::hexdump(GetBuffer(), GetLength());
}
void cByteBufferType::dumpBeginEnd()
{
    util::hexdumpBeginEnd(GetBuffer(), GetLength());
}
