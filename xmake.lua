-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/CommonLibVR")

-- set project
set_project("ad-mortem-permadeath")
set_version("1.0.1")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra", "error")
set_defaultmode("releasedbg")

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set policies
set_policy("package.requires_lock", true)

set_config("skyrim_vr", false)

add_requires("simpleini")

-- targets
target("ad-mortem-permadeath")
    -- add dependencies to target
    add_deps("commonlibsse-ng")

    add_packages("simpleini")

    -- add commonlibsse-ng plugin
    add_rules("commonlibsse-ng.plugin", {
        name = "ad-mortem-permadeath",
        author = "digital-apple",
        description = "Ad Mortem: A permadeath plugin"
    })

    -- add src files
    add_files("source/**.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include", { public = true })
    set_pcxxheader("include/PCH.h")

    -- copy build files to MODS or GAME paths (remove this if not needed)
    after_build(function(target)
        local copy = function(env, ext)
            for _, env in pairs(env:split(";")) do
                if os.exists(env) then
                    local plugins = path.join(env, ext, "SKSE/Plugins")
                    os.mkdir(plugins)
                    os.trycp(target:targetfile(), plugins)
                    os.trycp(target:symbolfile(), plugins)
                end
            end
        end
        if os.getenv("XSE_TES5_MODS_PATH") then
            copy(os.getenv("XSE_TES5_MODS_PATH"), target:name())
        elseif os.getenv("XSE_TES5_GAME_PATH") then
            copy(os.getenv("XSE_TES5_GAME_PATH"), "Data")
        end
    end)
