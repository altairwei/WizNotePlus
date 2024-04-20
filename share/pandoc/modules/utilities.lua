local module = {}

function module.concatArray(a, b)
  local result = {table.unpack(a)}
  table.move(b, 1, #b, #result + 1, result)
  return result
end

function module.replaceTable(tbl, start_index, end_index, new_element)
  -- Place the new element at the start index
  tbl[start_index] = new_element
  -- Calculate the number of elements to move
  local count = #tbl - end_index
  -- Move elements after the end_index to fill the previous space, if necessary
  if count > 0 then
    table.move(tbl, end_index + 1, #tbl, start_index + 1)
  end
  -- Adjust the size of the table, remove excess elements
  for i = #tbl, #tbl - (end_index - start_index) + 1, -1 do
    tbl[i] = nil
  end
end

return module