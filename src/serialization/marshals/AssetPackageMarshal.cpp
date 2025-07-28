/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#include <core/serialization/fbom/FBOM.hpp>
#include <core/serialization/fbom/FBOMArray.hpp>
#include <core/serialization/fbom/marshals/HypClassInstanceMarshal.hpp>

#include <core/object/HypData.hpp>

#include <core/utilities/Format.hpp>

#include <core/logging/Logger.hpp>
#include <core/logging/LogChannels.hpp>

#include <asset/AssetRegistry.hpp>

#include <HyperionEngine.hpp>

namespace hyperion::serialization {

template <>
class FBOMMarshaler<AssetPackage> : public HypClassInstanceMarshal
{
public:
    virtual ~FBOMMarshaler() override = default;

    virtual FBOMResult Serialize(ConstAnyRef in, FBOMObject& out) const override
    {
        if (FBOMResult err = HypClassInstanceMarshal::Serialize(in, out))
        {
            return err;
        }

        const AssetPackage& inObject = in.Get<AssetPackage>();

        for (const Handle<AssetPackage>& package : inObject.GetSubpackages())
        {
            if (!package.IsValid())
            {
                continue;
            }

            if (FBOMResult err = out.AddChild(*package))
            {
                return err;
            }
        }

        return { FBOMResult::FBOM_OK };
    }

    virtual FBOMResult Deserialize(FBOMLoadContext& context, const FBOMObject& in, HypData& out) const override
    {
        Handle<AssetPackage> assetPackageHandle = CreateObject<AssetPackage>();

        if (FBOMResult err = HypClassInstanceMarshal::Deserialize_Internal(context, in, AssetPackage::Class(), AnyRef(*assetPackageHandle)))
        {
            return err;
        }

        AssetPackageSet packages;

        for (const FBOMObject& child : in.GetChildren())
        {
            if (child.GetType().IsOrExtends("AssetPackage"))
            {
                const Handle<AssetPackage>& assetPackage = child.m_deserializedObject->Get<Handle<AssetPackage>>();

                if (!assetPackage)
                {
                    continue;
                }

                packages.Set(assetPackage);

                continue;
            }
        }

        assetPackageHandle->SetSubpackages(packages);

        out = HypData(std::move(assetPackageHandle));

        return { FBOMResult::FBOM_OK };
    }
};

HYP_DEFINE_MARSHAL(AssetPackage, FBOMMarshaler<AssetPackage>);

} // namespace hyperion::serialization