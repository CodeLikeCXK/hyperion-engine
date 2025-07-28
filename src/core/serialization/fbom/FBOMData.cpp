/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#include <core/serialization/fbom/FBOM.hpp>
#include <core/serialization/fbom/FBOMData.hpp>
#include <core/serialization/fbom/FBOMObject.hpp>
#include <core/serialization/fbom/FBOMConfig.hpp>
#include <core/serialization/fbom/FBOMWriter.hpp>
#include <core/serialization/fbom/FBOMReader.hpp>

#include <core/io/BufferedByteReader.hpp>
#include <core/io/ByteWriter.hpp>

#include <core/object/HypData.hpp>

#include <core/memory/Memory.hpp>

#include <core/json/JSON.hpp>

#include <sstream>

namespace hyperion::serialization {

FBOMData::FBOMData(EnumFlags<FBOMDataFlags> flags)
    : m_type(FBOMUnset()),
      m_flags(flags)
{
}

FBOMData::FBOMData(const FBOMType& type, EnumFlags<FBOMDataFlags> flags)
    : m_type(type),
      m_flags(flags)
{
    if (!type.IsUnbounded())
    {
        m_bytes.SetSize(type.size);
    }
}

FBOMData::FBOMData(const FBOMType& type, ByteBuffer&& byteBuffer, EnumFlags<FBOMDataFlags> flags)
    : m_bytes(std::move(byteBuffer)),
      m_type(type),
      m_flags(flags)
{
}

FBOMData::FBOMData(const FBOMData& other)
    : FBOMSerializableBase(static_cast<const FBOMSerializableBase&>(other)),
      m_type(other.m_type),
      m_bytes(other.m_bytes),
      m_flags(other.m_flags),
      m_deserializedObject(other.m_deserializedObject)
{
}

FBOMData& FBOMData::operator=(const FBOMData& other)
{
    if (this == &other)
    {
        return *this;
    }

    FBOMSerializableBase::operator=(static_cast<const FBOMSerializableBase&>(other));

    m_type = other.m_type;
    m_bytes = other.m_bytes;
    m_flags = other.m_flags;
    m_deserializedObject = other.m_deserializedObject;

    return *this;
}

FBOMData::FBOMData(FBOMData&& other) noexcept
    : FBOMSerializableBase(static_cast<FBOMSerializableBase&&>(std::move(other))),
      m_bytes(std::move(other.m_bytes)),
      m_type(std::move(other.m_type)),
      m_flags(other.m_flags),
      m_deserializedObject(std::move(other.m_deserializedObject))
{
    other.m_type = FBOMUnset();
    other.m_flags = FBOMDataFlags::NONE;
}

FBOMData& FBOMData::operator=(FBOMData&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    FBOMSerializableBase::operator=(static_cast<FBOMSerializableBase&&>(std::move(other)));

    m_bytes = std::move(other.m_bytes);
    m_type = std::move(other.m_type);
    m_flags = other.m_flags;
    m_deserializedObject = std::move(other.m_deserializedObject);

    other.m_type = FBOMUnset();
    other.m_flags = FBOMDataFlags::NONE;

    return *this;
}

FBOMData::~FBOMData()
{
}

FBOMResult FBOMData::ReadObject(FBOMLoadContext& context, FBOMObject& outObject) const
{
    if (!IsObject())
    {
        return { FBOMResult::FBOM_ERR, "Not an object" };
    }

    MemoryBufferedReaderSource source { m_bytes.ToByteView() };
    BufferedReader byteReader { &source };

    FBOMReader deserializer(FBOMReaderConfig {});

    // return deserializer.Deserialize(byteReader, outObject);
    if (FBOMResult err = deserializer.ReadObject(context, &byteReader, outObject, nullptr))
    {
        return err;
    }

    return {};
}

FBOMData FBOMData::FromObject(const FBOMObject& object, bool keepNativeObject)
{
    FBOMWriterConfig config;
    config.enableStaticData = false;

    MemoryByteWriter byteWriter;

    FBOMWriter serializer { config };

    if (FBOMResult err = object.Visit(&serializer, &byteWriter))
    {
        HYP_FAIL("Failed to serialize object: %s", err.message.Data());
    }

    FBOMData value = FBOMData(FBOMBaseObjectType(), std::move(byteWriter.GetBuffer()));
    AssertThrowMsg(value.IsObject(), "Expected value to be object: Got type: %s", value.GetType().ToString().Data());

    if (keepNativeObject)
    {
        value.m_deserializedObject = object.m_deserializedObject;
    }

    return value;
}

FBOMData FBOMData::FromObject(FBOMObject&& object, bool keepNativeObject)
{
    FBOMWriterConfig config;
    config.enableStaticData = false;

    MemoryByteWriter byteWriter;

    FBOMWriter serializer { config };

    if (FBOMResult err = object.Visit(&serializer, &byteWriter))
    {
        HYP_FAIL("Failed to serialize object: %s", err.message.Data());
    }

    FBOMData value = FBOMData(FBOMBaseObjectType(), std::move(byteWriter.GetBuffer()));
    AssertThrowMsg(value.IsObject(), "Expected value to be object: Got type: %s", value.GetType().ToString().Data());

    if (keepNativeObject)
    {
        value.m_deserializedObject = std::move(object.m_deserializedObject);
    }

    return value;
}

FBOMResult FBOMData::ReadArray(FBOMLoadContext& context, FBOMArray& outArray) const
{
    if (!IsArray())
    {
        return { FBOMResult::FBOM_ERR, "Not an array" };
    }

    MemoryBufferedReaderSource source { m_bytes.ToByteView() };
    BufferedReader byteReader { &source };

    FBOMReader deserializer(FBOMReaderConfig {});
    return deserializer.ReadArray(context, &byteReader, outArray);
}

FBOMData FBOMData::FromArray(const FBOMArray& array)
{
    FBOMWriterConfig config;
    config.enableStaticData = false;

    MemoryByteWriter byteWriter;

    FBOMWriter serializer { config };

    if (FBOMResult err = array.Visit(&serializer, &byteWriter))
    {
        HYP_FAIL("Failed to serialize array: %s", err.message.Data());
    }

    FBOMData value = FBOMData(FBOMArrayType(), std::move(byteWriter.GetBuffer()));
    AssertThrowMsg(value.IsArray(), "Expected value to be array: Got type: %s", value.GetType().ToString().Data());

    return value;
}

SizeType FBOMData::ReadBytes(SizeType n, void* out) const
{
    if (!m_type.IsUnbounded())
    {
        AssertThrowMsg(n <= m_bytes.Size(), "Attempt to read past max size of object");
    }

    SizeType toRead = MathUtil::Min(n, m_bytes.Size());
    Memory::MemCpy(out, m_bytes.Data(), toRead);
    return toRead;
}

ByteBuffer FBOMData::ReadBytes() const
{
    return m_bytes;
}

ByteBuffer FBOMData::ReadBytes(SizeType n) const
{
    if (!m_type.IsUnbounded())
    {
        AssertThrowMsg(n <= m_bytes.Size(), "Attempt to read past max size of object");
    }

    SizeType toRead = MathUtil::Min(n, m_bytes.Size());

    return ByteBuffer(toRead, m_bytes.Data());
}

void FBOMData::SetBytes(const ByteBuffer& byteBuffer)
{
    if (!m_type.IsUnbounded())
    {
        AssertThrowMsg(byteBuffer.Size() <= m_type.size, "Attempt to insert data past size max size of object (%llu > %llu)", byteBuffer.Size(), m_type.size);
    }

    m_bytes = byteBuffer;
}

void FBOMData::SetBytes(SizeType count, const void* data)
{
    if (!m_type.IsUnbounded())
    {
        AssertThrowMsg(count <= m_type.size, "Attempt to insert data past size max size of object (%llu > %llu)", count, m_type.size);
    }

    m_bytes.SetSize(count);
    m_bytes.SetData(count, data);
}

FBOMResult FBOMData::ToJSON(FBOMLoadContext& context, json::JSONValue& outJson) const
{
    if (IsInt8())
    {
        int8 value;

        if (FBOMResult err = ReadInt8(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsInt16())
    {
        int16 value;

        if (FBOMResult err = ReadInt16(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsInt32())
    {
        int32 value;

        if (FBOMResult err = ReadInt32(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsInt64())
    {
        int64 value;

        if (FBOMResult err = ReadInt64(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsUInt8())
    {
        uint8 value;

        if (FBOMResult err = ReadUInt8(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsUInt16())
    {
        uint16 value;

        if (FBOMResult err = ReadUInt16(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsUInt32())
    {
        uint32 value;

        if (FBOMResult err = ReadUInt32(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsUInt64())
    {
        uint64 value;

        if (FBOMResult err = ReadUInt64(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsFloat())
    {
        float value;

        if (FBOMResult err = ReadFloat(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsDouble())
    {
        double value;

        if (FBOMResult err = ReadDouble(&value))
        {
            return err;
        }

        outJson = json::JSONNumber(value);

        return {};
    }

    if (IsBool())
    {
        bool value;

        if (FBOMResult err = ReadBool(&value))
        {
            return err;
        }

        outJson = json::JSONBool(value);

        return {};
    }

    if (IsString())
    {
        String str;

        if (FBOMResult err = ReadString(str))
        {
            return err;
        }

        outJson = json::JSONString(std::move(str));

        return {};
    }

    if (IsArray())
    {
        FBOMArray array { FBOMUnset() };

        if (FBOMResult err = ReadArray(context, array))
        {
            return err;
        }

        json::JSONArray arrayJson;

        for (SizeType i = 0; i < array.Size(); i++)
        {
            json::JSONValue elementJson;

            if (FBOMResult err = array.GetElement(i).ToJSON(context, elementJson))
            {
                return err;
            }

            arrayJson.PushBack(std::move(elementJson));
        }

        outJson = std::move(arrayJson);

        return {};
    }

    if (IsObject())
    {
        FBOMObject object;

        if (FBOMResult err = ReadObject(context, object))
        {
            return err;
        }

        json::JSONObject objectJson;

        for (const auto& it : object.GetProperties())
        {
            json::JSONValue valueJson;

            if (FBOMResult err = it.second.ToJSON(context, valueJson))
            {
                return err;
            }

            objectJson[it.first] = std::move(valueJson);
        }

        outJson = std::move(objectJson);

        return {};
    }

    return { FBOMResult::FBOM_ERR, "Data could not be converted to JSON" };
}

FBOMData FBOMData::FromJSON(const json::JSONValue& jsonValue)
{
    if (jsonValue.IsNumber())
    {
        const bool isInteger = MathUtil::Floor<double, double>(jsonValue.AsNumber()) == jsonValue.AsNumber();

        if (isInteger)
        {
            return FBOMData::FromInt64(int64(jsonValue.AsNumber()));
        }
        else
        {
            return FBOMData::FromDouble(jsonValue.AsNumber());
        }
    }

    if (jsonValue.IsString())
    {
        return FBOMData::FromString(jsonValue.AsString().ToUTF8());
    }

    if (jsonValue.IsBool())
    {
        return FBOMData::FromBool(jsonValue.AsBool());
    }

    if (jsonValue.IsArray())
    {
        FBOMArray array { FBOMUnset() };

        const json::JSONArray& jsonArray = jsonValue.AsArray();

        if (jsonArray.Any())
        {
            Array<FBOMData> elements;
            elements.Reserve(jsonArray.Size());

            for (const json::JSONValue& element : jsonArray)
            {
                elements.EmplaceBack(FBOMData::FromJSON(element));
            }

            array = FBOMArray(elements[0].GetType(), std::move(elements));
        }

        return FBOMData::FromArray(std::move(array));
    }

    if (jsonValue.IsObject())
    {
        FBOMObject object;

        const json::JSONObject& jsonObject = jsonValue.AsObject();

        if (jsonObject.Any())
        {
            for (const Pair<json::JSONString, json::JSONValue>& pair : jsonObject)
            {
                object.SetProperty(ANSIString(pair.first), FBOMData::FromJSON(pair.second));
            }
        }

        return FBOMData::FromObject(std::move(object));
    }

    return FBOMData();
}

FBOMData::FBOMData(const json::JSONValue& jsonValue)
    : FBOMData(FromJSON(jsonValue))
{
}

FBOMResult FBOMData::Visit(UniqueID id, FBOMWriter* writer, ByteWriter* out, EnumFlags<FBOMDataAttributes> attributes) const
{
    return writer->Write(out, *this, id, attributes);
}

String FBOMData::ToString(bool deep) const
{
    std::stringstream stream;
    stream << "FBOM[";
    stream << "type: " << m_type.name << ", ";
    stream << "size: " << String::ToString(m_bytes.Size()) << ", ";
    stream << "data: { ";

    if (deep)
    {
        for (SizeType i = 0; i < m_bytes.Size(); i++)
        {
            stream << std::hex << int(m_bytes[i]) << " ";
        }
    }
    else
    {
        stream << m_bytes.Size();
    }

    stream << " } ";

    // if (next != nullptr) {
    //    stream << "<" << next->GetType().name << ">";
    //}

    stream << "]";

    return String(stream.str().c_str());
}

UniqueID FBOMData::GetUniqueID() const
{
    return UniqueID(GetHashCode());
}

HashCode FBOMData::GetHashCode() const
{
    HashCode hc;

    hc.Add(m_bytes.Size());
    hc.Add(m_type.GetHashCode());
    hc.Add(m_bytes.GetHashCode());

    return hc;
}

} // namespace hyperion::serialization
