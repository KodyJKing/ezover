Defs = {
    workspace = "PremakeTemplate",
    project = "PremakeTemplate"
}

-- Output definitions to a PowerShell script
local defsScript = io.open("scripts/definitions.ps1", "w")
for k, v in pairs(Defs) do
    local def = "$" .. k .. " = \"" .. v .. "\""
    print(def)
    defsScript:write(def .. "\n")
end
defsScript:close()

workspace(Defs.workspace)
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

    filter "platforms:Win32"
        system "Windows"
        architecture "x86"

    filter "platforms:Win64"
        system "Windows"
        architecture "x86_64"

outputdir = "%{cfg.buildcfg}-%{cfg.platform}"

project(Defs.project)
    kind "ConsoleApp"
    language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

    files { "src/**.h", "src/**.cpp" }

    includedirs { "src" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

