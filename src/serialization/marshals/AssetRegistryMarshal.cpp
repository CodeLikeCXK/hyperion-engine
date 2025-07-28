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
class FBOMMarshaler<AssetRegistry> : public HypClassInstanceMarshal
{
public:
    virtual ~FBOMMarshaler() override = default;

    virtual FBOMResult Serialize(ConstAnyRef in, FBOMObject& out) const override
    {
        if (FBOMResult err = HypClassInstanceMarshal::Serialize(in, out))
        {
            return err;
        }

        const AssetRegistry& inObject = in.Get<AssetRegistry>();

        for (const Handle<AssetPackage>& package : inObject.GetPackages())
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
        Handle<AssetRegistry> assetRegistryHandle = CreateObject<AssetRegistry>();

        if (FBOMResult err = HypClassInstanceMarshal::Deserialize_Internal(context, in, AssetRegistry::Class(), AnyRef(*assetRegistryHandle)))
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
            }
        }

        assetRegistryHandle->SetPackages(packages);

        out = HypData(std::move(assetRegistryHandle));

        return { FBOMResult::FBOM_OK };
    }
};

HYP_DEFINE_MARSHAL(AssetRegistry, FBOMMarshaler<AssetRegistry>);

} // namespace hyperion::serialization