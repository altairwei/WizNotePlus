local zip = require "pandoc.zip"
local html = require "wizhtml"
local logging = require "logging"

local function find(t, condition)
  for k, v in pairs(t) do
      if condition(v, k) then
          return v, k
      end
  end
  return nil
end

function ByteStringReader(input)
  local wizfile = zip.Archive(input)
  local indexent = find(wizfile.entries,
    function(ent) return ent.path == "index.html" end)

  if indexent then
    local indexfile = indexent:contents()
    local doc = pandoc.read(indexfile, "html", PANDOC_READER_OPTIONS)
    doc = doc:walk(html.WizHtmlFilter)
    --logging.temp(doc)
    return doc
  end

end
