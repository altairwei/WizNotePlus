local logging = require "logging"
local util = require "utilities"

local module = {}

function module.handle_wiz_div(elem)
  if elem.classes[1] == "wiz-code-container" then
    return module.handle_wiz_code_block(elem)
  end
end

-- Convert WizNote code blocks
function module.handle_wiz_code_block(elem)
  if elem.classes[1] == "wiz-code-container" then
    local codeLang = elem.attributes["mode"]
    local codes = {}
    elem:walk {
      Blocks = function(blocks)
        for _, bk in ipairs(blocks) do
          if bk.t == "CodeBlock" and bk.classes[1] == "CodeMirror-line" then
            codes[#codes + 1] = bk.text
          end
        end
      end
    }
    local codeblk = pandoc.CodeBlock(
      table.concat(codes, "\n"), { class = codeLang })
    return codeblk, false
  end
end

-- Convert WizNote nested lists
function module.handle_wiz_nested_list(elem)
  --[[
    The child of <OL> or <UL> should be <LI> tags, but WizNote makes the nested list
    be a child of <OL> or <UL>.
  --]]
  if elem.t == "OrderedList" or elem.t == "BulletList" then
    local children = elem.content
    -- Go from end to start to avoid problems with shifting indices.
    for i = #children-1, 1, -1 do
      local last_child = children[i+1][1]
      if last_child.t == "OrderedList" or last_child.t == "BulletList" then
        children[i] = util.concatArray(children[i], children[i+1])
        children:remove(i+1)
      end
    end
    return elem
  end
end

-- Convert WizNote TODO item
function module.handle_wiz_checkbox_item(elem)
  if elem.classes[1] == "wiz-todo-layer" then
    local todo = elem.content[1]
    todo = todo:walk {
      Span = function(span)
        return span.content
      end,
      Image = function(img)
        if img.classes[1] == "wiz-todo-checkbox" then
          if img.attributes["wiz-check"] == "unchecked" then
            return {pandoc.Str("☐"), pandoc.Space()}
          else
            return {pandoc.Str("☒"), pandoc.Space()}
          end
        end
      end
    }
    return todo
  end
end

-- Collect consecutive TODO items and convert them to a BulletList
function module.collect_wiz_todo_items(blocks)
  local inTodoList = false
  local output = {}
  local todolist = {}

  for i = 1, #blocks do
    local elem = blocks[i]

    -- Get TODO item inside blockquote
    if elem.t == "BlockQuote" and #elem.content == 1 then
      local inn = elem.content[1]
      if inn.classes and inn.classes[1] == "wiz-todo-layer" then
        elem = inn
      end
    end

    if elem.t == "Div" and elem.classes[1] == "wiz-todo-layer" then
      if not inTodoList then inTodoList = true end
      todolist[#todolist+1] = pandoc.Blocks(module.handle_wiz_checkbox_item(elem))
    elseif elem.t == "BlockQuote" and #todolist > 0 then
      todolist[#todolist] = util.concatArray(todolist[#todolist], elem.content)
    else
      if inTodoList then
        output[#output+1] = pandoc.BulletList(todolist)
        todolist = {}
        inTodoList = false
      end
      output[#output+1] = elem
    end
  end

  -- If todo list is not yet added
  if inTodoList then
    output[#output+1] = pandoc.BulletList(todolist)
  end

  return output
end

function module.remove_blockquote_as_ident(elem)
  if elem.t == "BlockQuote" then
    return pandoc.Div(elem.content)
  end
end

module.WizHtmlFilter = {
  traverse = "topdown",
  Blocks = module.collect_wiz_todo_items,
  Div = module.handle_wiz_div,
  OrderedList = module.handle_wiz_nested_list,
  BulletList = module.handle_wiz_nested_list,
  BlockQuote = module.remove_blockquote_as_ident
}

return module