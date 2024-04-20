local base = PANDOC_STATE["user_data_dir"]
local newPath1 = base .. "/modules/?.lua"
local newPath2 = base .. "/modules/?/init.lua"
package.path = table.concat({newPath1, newPath2, package.path}, ";")
