#ifndef C_JSON_VAR_H
#define C_JSON_VAR_H
/*
 * Contains cJSonVar class declaration.
 */

#include "common.h"


class cJSonVar;

const cJSonVar* ParseJSon(char *Buffer);

class cJSonVar
{
public:
    cJSonVar();
    ~cJSonVar();
    inline bool IsNull()const{return NullValue;}
    inline bool IsBool()const{return boolValue;}
    inline bool GetBoolValue()const{return bValue;}
    inline bool IsInteger()const{return integerValue;}
    inline int GetIntValue()const{return iValue;}
    inline bool IsArray()const{return Array;}
    inline bool IsVariable()const{return Value!=nullptr;}
    inline bool IsHash()const{return Hash;}
    inline const char* GetName()const{return Name;}
    inline uint32_t GetIndexPosition()const{return ArrayPosition;}
    inline const char* GetValue()const{return Value;}
    inline const cJSonVar* operator[](uint32_t Index)const{return GetByIndex(Index);}
    inline const cJSonVar* operator[](const char *Name)const{return Search(Name);}
    inline uint32_t GetArrayLen()const{return ArrayLen;}
    inline const cJSonVar* GetNext()const{return NextVar;}
    inline const cJSonVar* GetInternal()const{return InternalVarHead;}
    inline uint32_t GetInternalCount()const{return InternalVarCount;}
    const cJSonVar* Search(const char *Name)const;
    const cJSonVar* GetByIndex(uint32_t Index)const;
    void PrintSelf(FILE *f, bool addNewLine=true) const;
protected:
    const char *Name;
    const char *Value;
    uint32_t ArrayLen;
    //const char **Values;
    cJSonVar* NextVar;
    cJSonVar* InternalVarHead;
    uint32_t InternalVarCount;
    bool Array;
    bool Hash;
    bool NullValue;
    bool boolValue;
    bool bValue;
    bool integerValue;
    int  iValue;


    void CleanInternalVars();
    void CleanInternalArray();
    uint32_t ArrayPosition;
    inline void SetArrayPos(uint32_t ArrayLen){ArrayPosition=ArrayLen;}

    const char* ParseYourVar(char *&Buffer);
    const char* ParseHash(char *&Buffer);
    const char* ParseVar(char *&Buffer);
    const char* ParseArray(char *&Buffer);
    const char* ParseString(char *&Buffer,const char *&Var);
    const char* ParseValue(char *&Buffer);
    const char* ParseYourArrayValue(char *&Buffer);

    friend const cJSonVar* ParseJSon(char *Buffer);
};

#endif  // C_JSON_VAR_H
