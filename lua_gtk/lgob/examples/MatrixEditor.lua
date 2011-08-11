#! /usr/bin/env lua

require('lgob.gtk')

local Matrix    = {}
local MatrixMT  = {__index = Matrix}

function Matrix.new()
    local self = {}
    setmetatable(self, MatrixMT)
    
    self.iter = gtk.TreeIter.new()
    self.view = gtk.TreeView.new()
    self.view:set('enable-search', false, 'headers-visible', false)
    self.cols = {}
    
    self:change_order(10, 10)
    
    return self
end

function Matrix:cell_edited(path, new)
    local self, col = unpack(self)
    local row = tonumber(path)
    if col == 0 or row == 0 then return end
    
    self.model:get_iter_from_string(self.iter, path)
	self.model:set(self.iter, col, new)
end

function Matrix:format_cell(iter)
    local self, rend, col = unpack(self)
    local val, row        = self.model:get(iter, col, 0)
    rend:set( 'text', tostring(val) )
    
    if col == 0 or row == 0 then
        rend:set('background', '#AFAFAF')
    else
        rend:set('background', nil)
    end
end

function Matrix:change_order(m, n)
    self.view:set('model', nil)

    -- model columns
    local ctypes, vals = {'gint'}, {0}
    
    for i = 1, n do
        table.insert(ctypes, 'gdouble')
        table.insert(vals,  0)
    end
    
    self.model = gtk.ListStore.new( unpack(ctypes) )
    
    -- header row
    self.model:append(self.iter)
    for i = 1, n do self.model:set(self.iter, i, i) end
    
    -- model rows
    for i = 1, m do
        self.model:append(self.iter)
        self.model:seto(self.iter, unpack(vals) )
        self.model:set(self.iter, 0, i)
    end
    
    -- view columns (only add the columns that are needed
    for i = #self.cols, n do
        local rend = gtk.CellRendererText.new()
        rend:set    ('editable', true)
        rend:connect('edited', self.cell_edited, {self, i})
        
        local col  = gtk.TreeViewColumn.new_with_attributes(nil,rend,'text', i)
        col:set_cell_data_func(rend, self.format_cell, {self, rend, i})
        col:set('resizable', true)
        self.view:append_column(col)
        self.cols[i]  = col
    end
    
    self.view:set('model', self.model)
end

-- ** Main **

local win = gtk.Window.new()
win:set('title', 'Matrix Editor', 'window-position', gtk.WIN_POS_CENTER)

local mtx = Matrix.new()

win:add(mtx.view)
win:show_all()
win:connect('delete-event', gtk.main_quit) 

gtk.main()
