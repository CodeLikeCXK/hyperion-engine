using System;
using System.IO;
using System.Runtime.InteropServices;
using Hyperion;

namespace Hyperion
{
    [HypClassBinding(Name = "AssetPackageFlags")]
    [Flags]
    public enum AssetPackageFlags : uint
    {
        None = 0x0,
        Transient = 0x1,
        Hidden = 0x2
    }

    [HypClassBinding(Name = "AssetPackage")]
    public class AssetPackage : HypObject
    {
        private static LogChannel logChannel = LogChannel.ByName("Assset");

        public AssetPackage()
        {
        }
    }
}