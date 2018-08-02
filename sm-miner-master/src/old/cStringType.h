#ifndef STRINGOBJECT_H_INCLUDED
#define STRINGOBJECT_H_INCLUDED

#include "common.h"

#define GRANULARITY_SIZE(Need,Gran) ((Need)+(((Need)%(Gran))?((Gran)-((Need)%(Gran))):0))


class cStringType
{
protected:
    char *TextBuffer;
    size_t TextBufferSize;
    inline size_t CalculateBufferSize(size_t Need){return ((Need+1)+(((Need+1)%(32))?((32)-((Need+1)%(32))):0));}
public:
    cStringType();
    cStringType(size_t NewSize);
   // cStringType(cStringType&& srcStr);
    cStringType(const char *Text,bool Copy=false);
    cStringType(const cStringType &Obj,bool Copy=false);
    ~cStringType();
    void PrepareSize(size_t S=0);
    void OptimizeMemory();
    inline void Clear();
    const char *AssignStatic(const char *Text);
    inline operator const char *() const { return TextBuffer?TextBuffer:"";}
    inline const char* GetText() const { return TextBuffer?TextBuffer:"";}
    inline size_t GetBufferSize() const { return TextBufferSize;}
    inline char* GetBuffer(){return TextBufferSize>0?TextBuffer:NULL;}
    inline size_t GetLength()const{ return TextBuffer?strlen(TextBuffer):0;}
    inline char* GetBuffer(size_t NeedSize){PrepareSize(NeedSize);return TextBufferSize>0?TextBuffer:NULL;}
    inline bool operator == (const char *Text) const {return strcmp(TextBuffer?TextBuffer:"",Text)==0;}
    inline bool operator == (const cStringType &Str) const {return strcmp(GetText(),Str.GetText())==0;}
    inline bool IsEmpty()const {return TextBuffer?TextBuffer[0]=='\0':true;}
    inline operator bool ()const {return IsEmpty();}
    inline bool operator != (const char *Text)const {return strcmp(TextBuffer?TextBuffer:"",Text)!=0;}
    inline bool operator != (const cStringType &Str)const {return strcmp(GetText(),Str.GetText())!=0;}
    inline const char *operator =(const cStringType &Text){return (*this)=Text.GetText();}
    const char *operator =(const char *Text);
    const char *operator =(char Text);
    //void operator = (cStringType&& srcStr);
    const char *operator +=(const char *Text);
    inline const char *operator +=(cStringType &Text){return (*this)+=Text.GetText();}
    const char *operator +=(char Text);
    const char *operator +=(uint32_t Val);
    const char *operator +=(uint64_t Val);
    const char *operator +=(int32_t Val);
    const char *operator =(uint32_t Val);
    const char *operator =(uint64_t Val);
    const char *operator =(int32_t Val);
    const char *printf(const char *Str,...);
    const char *add_printf(const char *Str,...);
    bool EatEOL();

    static size_t strcopy(char *Dest,const char *Src,size_t Len);
    static char* EatSpaces(char *Dest);
    static const char* EatBeginSpaces(const char *Dest);
};

inline void cStringType::Clear()
{
    if(TextBuffer && TextBufferSize>0)
    {
        free(TextBuffer);
        TextBuffer=NULL;
        TextBufferSize=0;
    }
}


#endif // STRINGOBJECT_H_INCLUDED
