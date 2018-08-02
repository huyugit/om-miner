#ifndef C_BYTE_BUFFER_TYPE_H
#define C_BYTE_BUFFER_TYPE_H
/*
 * Contains cByteBufferType class declaration.
 */

#include "common.h"

#include "old/cStringType.h"


void DataToHex(const uint8_t *k,char *Str,uint32_t len,bool needend=true);

class cByteBufferType
{
public:
    static inline cByteBufferType fromUInt32(uint32_t data) { return cByteBufferType((uint8_t*)&data, sizeof(data)); }

    inline cByteBufferType(){InitBufferVars();}
    inline cByteBufferType(const uint8_t *val,uint32_t len){InitBufferVars();Assign(val,len);}
    inline cByteBufferType(const char *str){InitBufferVars();Assign(str);}
    cByteBufferType(const cByteBufferType& obj){InitBufferVars();Assign(obj);}
    ~cByteBufferType(){FreeBuffer();}
    void PrepareBuffer(uint32_t PrepareSize=0);
    inline void CleanBuffer(){Len=0;}
    inline void CleanBufferData(uint8_t v=0){if(Buffer){memset(Buffer,v,Len);}}
    inline void CleanAllBufferData(uint8_t v=0){if(Buffer){memset(Buffer,v,Size);}}
    inline void FreeBuffer(){if(Buffer){free(Buffer);Buffer=nullptr;Size=Len=0;}}
    inline bool IsEmpty()const{return Len==0;}
    inline bool IsPresent()const{return Len>0;}
    inline uint32_t GetLength()const{return Len;}
    inline uint32_t GetNeedStringLength()const{return Len*2;}
    inline uint32_t GetBufferSize()const{return Size;}
    inline const uint8_t* GetBuffer()const{return Buffer;}
    inline operator const uint8_t*()const{return Buffer;}
    inline operator uint8_t*(){return Buffer;}
    inline uint8_t* GetBuffer(){return Buffer;}
    inline uint8_t* GetBuffer(uint32_t S){PrepareBuffer(S);return Buffer;}
    inline void SetLength(uint32_t S){PrepareBuffer(S);Len=S;}
    inline uint8_t& operator[](uint32_t index){return Buffer[index];}
    inline uint8_t operator[](uint32_t index)const {return Buffer[index];}
    inline void Assign(const uint8_t *val,uint32_t l){CleanBuffer();PrepareBuffer(l);if(l>0){memcpy(Buffer,val,l);Len=l;}}
    inline void Concat(const uint8_t *val,uint32_t l){PrepareBuffer(Len+l);if(l>0){memcpy(&Buffer[Len],val,l);Len+=l;}}
    inline void Assign(const char *str){CleanBuffer();uint32_t l=strlen(str)/2;PrepareBuffer(l);if(l>0){Len=HexToBuff(str,Buffer,l);}}
    inline void Concat(const char *str){uint32_t l=strlen(str)/2;PrepareBuffer(Len+l);if(l>0){Len+=HexToBuff(str,Buffer,l);}}
    inline void Assign(const cByteBufferType& obj){CleanBuffer();PrepareBuffer(obj.GetLength());if(obj.Len>0){memcpy(Buffer,obj.Buffer,obj.Len);Len=obj.Len;}}
    inline void Concat(const cByteBufferType& obj){PrepareBuffer(Len+obj.Len);if(obj.Len>0){memcpy(&Buffer[Len],obj.Buffer,obj.Len);Len+=obj.Len;}}
    inline cByteBufferType& operator=(const cByteBufferType& obj){Assign(obj);return *this;}
    inline cByteBufferType& operator+=(const cByteBufferType& obj){Concat(obj);return *this;}
    inline cByteBufferType operator+(const cByteBufferType& obj){cByteBufferType k(*this);k+=obj;return k;}
    inline cByteBufferType operator+(const char *str){cByteBufferType k(*this);k+=str;return k;}
    inline bool operator==(const cByteBufferType& obj){return Len==obj.Len && memcmp(Buffer,obj.Buffer,Len)==0;}
    inline cByteBufferType& move(cByteBufferType& obj){FreeBuffer();Buffer=obj.Buffer;Size=obj.Size;Len=obj.Len;obj.Buffer=nullptr;obj.Size=obj.Len=0;return *this;}
    inline bool operator==(const uint8_t* obj){return Len>0 && memcmp(Buffer,obj,Len)==0;}
    inline bool operator==(const char* str){cByteBufferType k(str);return (*this)==k;}
    void swap_array();
    void SwapDWORDEndians();

    static uint32_t HexToBuff(const char *Str,uint8_t *res,uint32_t len);
    inline cStringType ToText()const{cStringType str;str.PrepareSize(Len*2);DataToHex(Buffer,str.GetBuffer(),Len);return str;}

    void dump();
    void dumpBeginEnd();
protected:
    uint8_t *Buffer;
    uint32_t Size;
    uint32_t Len;

    inline uint32_t CalculateBufferSize(uint32_t Need){return ((Need)+(((Need)%(32))?((32)-((Need)%(32))):0));}
    inline void InitBufferVars(){Buffer=nullptr;Size=0;Len=0;}
};

#endif  // C_BYTE_BUFFER_TYPE_H
