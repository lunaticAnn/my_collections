local DEFAULT_CACHING = 10
local bottles = {}
local mostRecentId = 1
local leastRecentId = 1
local pause = false
local lock = false
local DecreaseButton = script.Parent.NumberOfCache.Decrease
local IncreaseButton = script.Parent.NumberOfCache.Increase
local NumberLabel = script.Parent.NumberOfCache.Number

local function resetQueue()
	for _,bottle in pairs(bottles) do
		bottle:Destroy()
	end
	bottles = {}
	mostRecentId = 1
	leastRecentId = 1
end

local function drawSegment(start_x, start_y, end_x, end_y, _color)
	local screenY = script.Parent.AbsoluteSize.Y
	start_y = screenY - start_y
	end_y = screenY - end_y
	local dx = end_x - start_x
	local dy = end_y - start_y
	local len = math.sqrt(dx * dx + dy * dy)
	local segment = script.Parent.LineSegment:Clone()
	segment.Size = UDim2.new(0, len, 0, 1)
	segment.Position = UDim2.new(0, (start_x + end_x ) / 2, 0, (start_y + end_y) / 2)
	if dx == 0 then 
		segment.Rotation = 90
	else
		local rot = (math.atan(dy / dx) / math.pi) * 180
		segment.Rotation = rot
	end
	if _color then
		segment.BackgroundColor3 = _color
	end
	return segment
end

local function getPosition(r, c_x, c_y, theta)
	local x = c_x + math.cos(theta) * r
	local y = c_y + math.sin(theta) * r
	return {
		x = x,
		y = y	
	}
end
local function drawCurve(r, c_x, c_y, startTheta, endTheta, interpolateNumber, _parent, _startColor, _endColor, _hideR)
	local nodeList = {}
	for i = 1, interpolateNumber + 1 do
		-- calculate interpolated radius
		local theta = startTheta + (endTheta - startTheta) / interpolateNumber * (i - 1)
		nodeList[i] = getPosition(r, c_x, c_y, theta)
	end
	for i = 1, interpolateNumber do
		local segment = drawSegment(nodeList[i].x, nodeList[i].y, nodeList[i+1].x, nodeList[i+1].y)
		segment.Parent = _parent
		if _startColor then
			if _endColor then
				local t = i / interpolateNumber
				segment.BackgroundColor3 = _startColor:lerp(_endColor, t)
			else
				segment.BackgroundColor3 = _startColor
			end
		end
	end
	if not _hideR then
		local segment = drawSegment(c_x, c_y, nodeList[1].x, nodeList[1].y,_startColor)
		segment.Parent = _parent
		segment = drawSegment(c_x, c_y, nodeList[interpolateNumber + 1].x, nodeList[interpolateNumber + 1].y, _endColor)
		segment.Parent = _parent
	end
end

-- calculate the 3 centers of curves
local function createBottle(contact_x, contact_y, rmax, rmin, theta, rollTheLarger, _color1, _color2)
	local bottle = Instance.new("Folder")
	bottle.Name = "Bottle"
	bottle.Parent = script.Parent
	local seg = drawSegment(0, contact_y, 2 * contact_x, contact_y, Color3.new(0,1,1))
	seg.Parent = bottle
	seg = drawSegment(0, contact_y + rmax + rmin, 2 * contact_x, contact_y + rmax + rmin, Color3.new(0,0,1))
	seg.Parent = bottle

	local c1_x, c1_y, c2_x, c2_y, c3_x, c3_y
	local thetas = {}
	if rollTheLarger then
		c1_x = contact_x
		c1_y = contact_y + rmax
		c2_x = contact_x + (rmax - rmin)* math.sin(theta)
		c2_y = contact_y + rmax - (rmax - rmin) * math.cos(theta)
		c3_x = contact_x - (rmax - rmin) * math.sin(math.pi/3 - theta)
		c3_y = contact_y + rmax - (rmax - rmin) * math.cos(math.pi/3 - theta)
		thetas[1] = theta - math.pi * 5 / 6
	else
		c1_x = contact_x - (rmax - rmin)* math.sin(theta)
		c1_y = contact_y + rmin + (rmax - rmin) * math.cos(theta)
		c2_x = contact_x + (rmax - rmin) * math.sin(math.pi/3 - theta)
		c2_y = contact_y + rmin + (rmax - rmin) * math.cos(math.pi/3 - theta)
		c3_x = contact_x
		c3_y = contact_y + rmin
		thetas[1] = theta - math.pi / 2
	end
	for i = 2, 7 do
		thetas[i] = thetas[1] + math.pi * (i - 1) / 3
	end
	drawCurve(rmax, c1_x, c1_y, thetas[1], thetas[2], 20, bottle, _color1, _color2, true)
	drawCurve(rmin, c2_x, c2_y, thetas[2], thetas[3], 20, bottle, _color2, _color1, true)
	drawCurve(rmax, c3_x, c3_y, thetas[3], thetas[4], 20, bottle, _color1, _color2, true)
	drawCurve(rmin, c1_x, c1_y, thetas[4], thetas[5], 20, bottle, _color2, _color1, true)
	drawCurve(rmax, c2_x, c2_y, thetas[5], thetas[6], 20, bottle, _color1, _color2, true)
	drawCurve(rmin, c3_x, c3_y, thetas[6], thetas[7], 20, bottle, _color2, _color1, true)
	return bottle
end
local function changeCacheNumber(increment)
	if not lock then
		lock = true
		pause = true
		DEFAULT_CACHING  = DEFAULT_CACHING + increment
		if DEFAULT_CACHING < 1 then
			DEFAULT_CACHING = 1
		end
		NumberLabel.Text = tostring(DEFAULT_CACHING)
		-- reset the old queue
		resetQueue()
		pause = false
		lock = false
	end
end

DecreaseButton.Activated:Connect(function()
	print("Decrease activated")
	changeCacheNumber(-1)
end)

IncreaseButton.Activated:Connect(function()
	changeCacheNumber(1)
end)

local i = 1
local j = true
local c0 = Color3.new(1,0,0)
local c1 = Color3.new(1,1,0)
local c2 = Color3.new(0,1,1)
while 1 do
	if not pause then
		i = i + 1
		if i == 31 then
			i = 1
			j = not j
		end
		local t = i / 30
		bottles[mostRecentId] = createBottle(300, 50, 200, 40, math.pi * i / 90, j, c1:lerp(c0, t), c2:lerp(c0, t))
		mostRecentId = mostRecentId + 1
		if mostRecentId - leastRecentId > DEFAULT_CACHING then
			local toRemove = bottles[leastRecentId]
			bottles[leastRecentId] = nil
			leastRecentId = leastRecentId + 1
			toRemove:Destroy()
		end
	end
	wait()
end
