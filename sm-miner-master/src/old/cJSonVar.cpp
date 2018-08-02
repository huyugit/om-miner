/*
 * Contains cJSonVar class definition.
 */

#include "cJSonVar.h"


inline char* EatEmpty(char *str)
{
    while(*str==' ' || *str=='\t') str++;
    return str;
}


const cJSonVar* ParseJSon(char *Buffer)
{
    cJSonVar* obj=new cJSonVar();
    size_t len=strlen(Buffer);
    char *Begin=Buffer;
    const char *res=obj->ParseYourVar(Buffer);
    if(res)
    {
        //printf("Some error near %lu (%20s)\n",res-Begin,res);
        printf("Some JSon error near pos %d, total len %zu, from text [%.20s]\n",res-Begin,len,res);
        delete obj;
        obj=nullptr;
    }
    return obj;
}

cJSonVar::cJSonVar()
:Name(nullptr),Value(nullptr),ArrayLen(0)
,NextVar(nullptr),InternalVarHead(nullptr),InternalVarCount(0)
,Array(false),Hash(false),NullValue(false),boolValue(false),bValue(false)
,integerValue(false),iValue(0),ArrayPosition(0)
{
}

cJSonVar::~cJSonVar()
{
    if(InternalVarHead)
    {
        CleanInternalVars();
    }
    CleanInternalArray();
}

void cJSonVar::PrintSelf(FILE *f, bool addNewLine) const
{
    if(GetName())
        fprintf(f,"%s=",GetName());
    if(IsVariable())
    {
        fprintf(f,"\"%s\"",GetValue());
    }else
    {
        if(IsNull())
            fprintf(f,"NULL");
        if(IsArray())
        {
            fprintf(f,"(%u)[",GetArrayLen());
            /*for(uint32_t i=0,len=GetArrayLen();i<len;i++)
            {
            fprintf(f,"%s%s",Values[i],i+1==len?"":",");
            }*/
            const cJSonVar *obj=GetInternal();
            while(obj)
            {
                obj->PrintSelf(f, false);
                obj=obj->GetNext();
                if(obj)
                    fprintf(f, ",");
            }
            fprintf(f,"]");
        }
        if(IsHash())
        {
            fprintf(f,"(%u){",GetInternalCount());
            const cJSonVar *obj=GetInternal();
            while(obj)
            {
                obj->PrintSelf(f, false);
                obj=obj->GetNext();
                if(obj)
                    fprintf(f,",");
            }
            fprintf(f,"}");
        }
        if(IsBool())
        {
            fprintf(f,"%s",GetBoolValue()?"true":"false");
        }
        if(IsInteger())
        {
            fprintf(f,"%d",GetIntValue());
        }
    }
    if (addNewLine) {
        fprintf(f, "\n");
    }
}

const char* cJSonVar::ParseString(char *&Buffer,const char *&Var)
{
    if(*Buffer!='"')
        return Buffer;
    Buffer++;
    Var=Buffer;
    while(*Buffer!='"' && *Buffer!='\0')
    {
        Buffer++;
    }
    if(*Buffer=='"')
    {
        *Buffer='\0';
        Buffer++;
    }
    else
        return Buffer;
    return nullptr;
}

const char* cJSonVar::ParseValue(char *&Buffer)
{
    const char *Var=Buffer;
    while(*Buffer!='\0')
    {
        if(*Buffer==',' || *Buffer==']' || *Buffer=='}')
        {
            char SaveVar=*Buffer;
            *Buffer='\0';
            if(sscanf(Var,"%d",&iValue)!=1)
                return Var;
            *Buffer=SaveVar;
            integerValue=true;
            break;
        }
        Buffer++;
    }
    if(*Buffer=='\0')
        return Buffer;
    return nullptr;
}

const char* cJSonVar::ParseVar(char *&Buffer)
{
    if(*Buffer!='"')
        return Buffer;
    const char* res;
    if((res=ParseString(Buffer,Name)))
        return res;
    Buffer=EatEmpty(Buffer);
    if(*Buffer!=':')
    {
        return Buffer;
    }
    Buffer++;
    Buffer=EatEmpty(Buffer);
    if(*Buffer=='{')
        return ParseHash(Buffer);
    if(*Buffer=='"')
    {
        if((res=ParseString(Buffer,Value)))
            return res;
        return nullptr;
    }
    if(*Buffer=='[')
    {
        if((res=ParseArray(Buffer)))
            return res;
        return nullptr;
    }
    if(strncmp(Buffer,"null",4)==0)
    {
        Buffer+=4;
        Buffer=EatEmpty(Buffer);
        NullValue=true;
        return nullptr;
    }
    if(strncmp(Buffer,"false",5)==0)
    {
        Buffer+=strlen("false");
        Buffer=EatEmpty(Buffer);
        boolValue=true;
        bValue=false;
        return nullptr;
    }
    if(strncmp(Buffer,"true",4)==0)
    {
        Buffer+=strlen("true");
        Buffer=EatEmpty(Buffer);
        boolValue=true;
        bValue=true;
        return nullptr;
    }
    if(*Buffer=='-' || *Buffer=='+' || (*Buffer>='0' && *Buffer<='9'))
    {
        if((res=ParseValue(Buffer)))
            return res;
        return nullptr;
    }
    return Buffer;
}

const char* cJSonVar::ParseArray(char *&Buffer)
{
    CleanInternalArray();
    if(*Buffer!='[')
        return Buffer;
    Buffer++;
    Array=true;
    const char* res;
    /*uint32_t TempValuesSize=16;
    const char **TempValues=nullptr;
    //new (const char*)[TempValuesLen];
    Values=new const char*[TempValuesSize];
    */
    Buffer=EatEmpty(Buffer);
    CleanInternalArray();
    cJSonVar *Tail=nullptr;
    while(*Buffer!=']' && *Buffer!='\0')
    {
        cJSonVar *obj=new cJSonVar();
        obj->SetArrayPos(ArrayLen);
        if(!Tail)
        {
            Tail=obj;
            ArrayLen=1;
            InternalVarHead=obj;
        }else
        {
            Tail->NextVar=obj;
            Tail=obj;
            ArrayLen++;
        }
        if((res=obj->ParseYourArrayValue(Buffer)))
            return res;
        Buffer=EatEmpty(Buffer);
        if(*Buffer==',')
        {
            Buffer++;
            Buffer=EatEmpty(Buffer);
            if(*Buffer=='\0')
                return Buffer;
            continue;
        }
        Buffer=EatEmpty(Buffer);
        if(*Buffer!=']')
            return Buffer;
    }
    Buffer=EatEmpty(Buffer);
    if(*Buffer==']')
    {
        Buffer++;
    }
    else
        return Buffer;
    return nullptr;
}

const char* cJSonVar::ParseHash(char *&Buffer)
{
    if(*Buffer!='{')
        return Buffer;
    Buffer++;
    Hash=true;
    const char* res;
    CleanInternalVars();
    cJSonVar *Tail=nullptr;
    while(*Buffer!='}' && *Buffer!='\0')
    {
        Buffer=EatEmpty(Buffer);
        if(*Buffer!='"')//name var expected
            return Buffer;
        cJSonVar *obj=new cJSonVar();
        if(!Tail)
        {
            Tail=obj;
            InternalVarHead=obj;
            InternalVarCount=1;
        }else
        {
            Tail->NextVar=obj;
            Tail=obj;
            InternalVarCount++;
        }
        if((res=obj->ParseYourVar(Buffer)))
            return res;
        Buffer=EatEmpty(Buffer);
        if(*Buffer==',')
        {
            Buffer++;
            Buffer=EatEmpty(Buffer);
            if(*Buffer=='\0')
                return Buffer;
            continue;
        }
        Buffer=EatEmpty(Buffer);
        if(*Buffer!='}')
            return Buffer;
    }
    Buffer=EatEmpty(Buffer);
    if(*Buffer=='}')
        Buffer++;
    else
        return Buffer;
    return nullptr;
}

const char* cJSonVar::ParseYourVar(char *&Buffer)
{
    Buffer=EatEmpty(Buffer);
    if(*Buffer=='{')
        return ParseHash(Buffer);
    if(*Buffer=='"')
        return ParseVar(Buffer);
    if(*Buffer=='[')
        return ParseArray(Buffer);
    return Buffer;
}

const char* cJSonVar::ParseYourArrayValue(char *&Buffer)
{
    Buffer=EatEmpty(Buffer);
    if(*Buffer=='{')
        return ParseHash(Buffer);
    if(*Buffer=='"')
        return ParseString(Buffer,Value);
    if(*Buffer=='[')
        return ParseArray(Buffer);
    if(strncmp(Buffer,"null",4)==0)
    {
        Buffer+=4;
        Buffer=EatEmpty(Buffer);
        NullValue=true;
        return nullptr;
    }
    if(strncmp(Buffer,"false",5)==0)
    {
        Buffer+=strlen("false");
        Buffer=EatEmpty(Buffer);
        boolValue=true;
        bValue=false;
        return nullptr;
    }
    if(strncmp(Buffer,"true",4)==0)
    {
        Buffer+=strlen("true");
        Buffer=EatEmpty(Buffer);
        boolValue=true;
        bValue=true;
        return nullptr;
    }
    if(*Buffer=='-' || *Buffer=='+' || (*Buffer>='0' && *Buffer<='9'))
    {
        const char *res;
        if((res=ParseValue(Buffer)))
            return res;
        return nullptr;
    }
    return Buffer;
}

void cJSonVar::CleanInternalVars()
{
    cJSonVar *obj;
    while(InternalVarHead)
    {
        obj=InternalVarHead;
        InternalVarHead=InternalVarHead->NextVar;
        delete obj;
    }
    InternalVarCount=0;
    ArrayLen=0;
}

void cJSonVar::CleanInternalArray()
{
    cJSonVar *obj;
    while(InternalVarHead)
    {
        obj=InternalVarHead;
        InternalVarHead=InternalVarHead->NextVar;
        delete obj;
    }
    InternalVarCount=0;
    ArrayLen=0;
}

const cJSonVar* cJSonVar::GetByIndex(uint32_t Index)const
{
    const cJSonVar* obj=GetInternal();
    while(obj)
    {
        if(obj->GetIndexPosition()==Index)
            return obj;
        obj=obj->GetNext();
    }
    return nullptr;
}

const cJSonVar* cJSonVar::Search(const char *Name)const
{
    const cJSonVar* obj=GetInternal();
    while(obj)
    {
        if(strcmp(obj->GetName(),Name)==0)
            return obj;
        obj=obj->GetNext();
    }
    return nullptr;
}
