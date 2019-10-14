local LOOP_SEGMENT_NUM = 30
local sin = math.sin
local cos = math.cos
local pi = math.pi
local indexerPrefab = script.Indexer
local beamPrefab = script.BeamPrefab

--[[
	LoopEdge{
		Name = indexToString
		ParentPart = indexer,
		Children = {} -- ordered children vertices	
	}	
]]
local function createCirclePoints(_index --[[int]], _radiusa--[[float]], _center --[[Vector3]], _radiusb, dir)
	-- create circle around center
	if not _radiusb then
		_radiusb = _radiusa
	end
	local indexer = indexerPrefab:Clone()
	indexer.Name = tostring(_index)
	local children = {}
	for i = 1, LOOP_SEGMENT_NUM do
		local point = Instance.new("Attachment", indexer)
		point.Name = tostring(i)
		local a = _radiusa * cos(pi * 2 / LOOP_SEGMENT_NUM * i)
		local b = _radiusb * sin(pi * 2 / LOOP_SEGMENT_NUM * i)
		
		local cframe = CFrame.new(0,0,0)
		if dir then cframe = CFrame.new(Vector3.new(0,0,0), dir) end
		point.WorldPosition = _center + cframe.LookVector * a + cframe.RightVector * b
		children[#children + 1] = point
	end
	return { 
		Name = tostring(_index), 
		ParentPart = indexer, 
		Children = children 
	}
end

local function createEdges(_index --[[return table of attachment group]], _color)
	local edgeGroup = Instance.new("Folder")
	edgeGroup.Name = "LoopEdges".._index.Name
	for i = 1, LOOP_SEGMENT_NUM do
		local e = beamPrefab:Clone()
		e.Parent = edgeGroup
		if _color then
			e.Color = ColorSequence.new(_color)
		end
		e.Attachment0 = _index.Children[i]
		if i < LOOP_SEGMENT_NUM then
			e.Attachment1 = _index.Children[i + 1]
		else
			e.Attachment1 = _index.Children[1]
		end	
	end
	return edgeGroup
end

local function createInterEdges(_index1, _index2, _color1, _color2)
	local edgeGroup = Instance.new("Folder")
	edgeGroup.Name = "InterEdges".._index1.Name.._index2.Name
	for i = 1, LOOP_SEGMENT_NUM do
		local e = beamPrefab:Clone()
		e.Parent = edgeGroup
		if _color1 and _color2 then
			e.Color = ColorSequence.new(_color1, _color2)
		end
		e.Attachment0 = _index1.Children[i]
		e.Attachment1 = _index2.Children[i]
	end
	return edgeGroup
end

--[[ 
create burger instantiate a burger with a list of loop defines and returns the cache
	i.e. {
		{ radius = 40, center = Vector3.new(0,10,0), dir = Vector3(1,0,0)},
		{ radius = 40, center = Vector3.new(0,10,0)},	
	}		
]]
local function createBurger(inList, root)
	local cache = {}
	local edges = Instance.new("Folder", root)
	edges.Name = "edges"
	for i  = 1, #inList do
		local indexer = createCirclePoints(i, inList[i].radius, inList[i].center, inList[i].radiusy, inList[i].dir)
		indexer.ParentPart.Parent = root
		local edgeGroup = createEdges(indexer, inList[i].color)
		edgeGroup.Parent = edges
		cache[#cache + 1] = indexer
		if i > 1 then
			local interEdgeGroup = createInterEdges(cache[i - 1], cache[i], inList[i-1].color, inList[i].color)
			interEdgeGroup.Parent = edges
		end
	end
	return cache
end

local WAVE_FACTOR = 0.02
local WAVE_SPD = 5
local function wave(cache)
	for i = 1, #cache do
		local children = cache[i].Children
		for j = 1, #children do 
			local current = children[j].WorldPosition
			local waveForce = WAVE_FACTOR * sin(WAVE_SPD * time() + current.x + current.z)
			children[j].WorldPosition = current + Vector3.new(0, waveForce, 0)
		end
	end
end

local function scaleBurgerY(_burger, scale)
	for i = 1, #_burger do
		_burger[i].center = _burger[i].center * scale
	end
end

local function moveBurger(_burger, translation)
	for i = 1, #_burger do
		_burger[i].center = _burger[i].center + translation
	end
end

local function scaleBurgerB(_burger, scale)
	for i = 1, #_burger do
		_burger[i].radiusy = _burger[i].radiusy and _burger[i].radiusy * scale or _burger[i].radius * scale
	end
end

local function scaleBurgerA(_burger, scale)
	for i = 1, #_burger do
		_burger[i].radius = _burger[i].radius * scale
	end
end

local defaultBurger = {
	{ radius = 1, center = Vector3.new(0,21,0), color = Color3.new(1, 1, 1)},
	{ radius = 30, center = Vector3.new(0,20,0), color = Color3.new(1, 0, 0)},
	{ radius = 35, center = Vector3.new(0,18,0), color = Color3.new(1, 0, 0)},
	{ radius = 38, center = Vector3.new(0,15,0), color = Color3.new(1, 1, 0)},
	{ radius = 40, center = Vector3.new(0,10,0), color = Color3.new(1, 1, 0)},
	{ radius = 33, center = Vector3.new(0,9,0), color = Color3.new(1, 1, 0)},
	{ radius = 35, center = Vector3.new(0,8,0), color = Color3.new(0, 1, 0)},
	{ radius = 37, center = Vector3.new(0,7,0), color = Color3.new(0, 1, 0)},
	{ radius = 37, center = Vector3.new(0,3,0), color = Color3.new(0, 1, 0)},
	{ radius = 35, center = Vector3.new(0,2,0), color = Color3.new(0, 1, 1)},
	{ radius = 33, center = Vector3.new(0,1,0), color = Color3.new(0, 1, 1)},
	{ radius = 40, center = Vector3.new(0,0,0), color = Color3.new(0, 0, 1)},
	{ radius = 40, center = Vector3.new(0,-5,0), color = Color3.new(0, 0, 1)},
	{ radius = 38, center = Vector3.new(0,-7,0), color = Color3.new(1, 0, 1)},
	{ radius = 1, center = Vector3.new(0,-7.5,0), color = Color3.new(1, 1, 1)},
}

local defaultFish = {
	{ radius = 1, center = Vector3.new(24,0,0), color = Color3.new(1, 1, 1), dir = Vector3.new(0, 1, 0)},
	{ radius = 15, center = Vector3.new(20,0,0), color = Color3.new(1, 0, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 17, center = Vector3.new(18,0,0), color = Color3.new(1, 0, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 19, center = Vector3.new(15,0,0), color = Color3.new(1, 1, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 20, center = Vector3.new(10,0,0), color = Color3.new(1, 1, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 15, center = Vector3.new(5,0,0), color = Color3.new(1, 1, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 3, center = Vector3.new(3,0,0), color = Color3.new(1, 1, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 12, center = Vector3.new(0,0,0), color = Color3.new(0, 1, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 15, center = Vector3.new(-3,0,0), color = Color3.new(0, 1, 0), dir = Vector3.new(0, 1, 0)},
	{ radius = 1, center = Vector3.new(-3,0,0), color = Color3.new(0, 1, 0), dir = Vector3.new(0, 1, 0)},
}

scaleBurgerY(defaultBurger, 1.3)

--scaleBurgerY(defaultFish, 1.5)
scaleBurgerB(defaultFish, 0.1)
scaleBurgerA(defaultFish, 0.5)

local cache = createBurger(defaultBurger, workspace.Burger)
local cacheFish = createBurger(defaultFish, workspace.Fish)
local runService = game:GetService("RunService")
runService.Heartbeat:connect(function()
	wave(cache)
	wave(cacheFish)
end)


