function printTable(t)
    for _,r in pairs(t) do
        print(table.unpack(r))
    end
end

function simplex(A, b, c)
	-- Add slack variables to convert inequalities to equalities
	local m, n = #A, #c
	local slack = {}
	for i = 1, m do
		slack[i] = {}
		for j = 1, m do
			slack[i][j] = (i == j) and 1 or 0
		end
	end
	local tableau = {}
	for i = 1, m do
		tableau[i] = {}
		for j = 1, n + m do
			if j <= n then
				tableau[i][j] = A[i][j]
			else				
				tableau[i][j] = slack[i][j - n]
			end
		end
		tableau[i][n + m + 1] = b[i]
	end

    -- coefficient row
	local co = {}
	for i = 1, n + m + 1 do
		if i <= n then
			co[i] = -c[i]
		else
			co[i] = 0
		end
	end

	-- Initialize the basic and non-basic variables
	local B = {}
	for i = 1, m do
		B[i] = n + i
	end
	local N = {}
	for i = 1, n do
		N[i] = i
	end

	while true do
        -- check coeffiecient row, find entering variable j
        local min_delta, j = math.huge, 0
        for i = 1, n + m do
            if co[i] < min_delta then
                min_delta, j = co[i], i
            end
        end
        if min_delta >= 0 then
            break
        end
        
        -- Choose the leaving variable i (i_th basic)
        local i, min_ratio = 0, math.huge
        for k = 1, m do
            if tableau[k][j] > 0 then
                local ratio = tableau[k][n + m + 1] / tableau[k][j]
                if ratio < min_ratio then
                    i, min_ratio = k, ratio
                end
            end
        end

		-- Pivot
		local pivot = tableau[i][j]
		
		-- normamlize row
		for k = 1, n + m + 1 do
			tableau[i][k] = tableau[i][k] / pivot
		end
		
		--Guassian elimination
		for k = 1, m do
			if k ~= i then
				local factor = tableau[k][j]
				for l = 1, n + m + 1 do
					tableau[k][l] = tableau[k][l] - factor * tableau[i][l]
				end
			end
		end
		
		for l = 1, n + m + 1 do
			co[l] = co[l] - min_delta * tableau[i][l]
		end
		
		-- Update basic and non-basic variables, swap
		local bi = B[i]
		B[i] = j
		
		for k, v in pairs(N) do
			if v == j then
				N[k] = bi
				break
			end
		end
	end

	-- Extract the optimal solution and objective function value
	local opt_sol = {}
	for i = 1, n + m do
		opt_sol[i] = 0
	end
	for i = 1, m do
		opt_sol[B[i]] = tableau[i][n + m + 1]
	end

	local opt_val = 0
	for i = 1, n do
		opt_val = opt_val + c[i] * opt_sol[i]
	end

	return opt_sol, opt_val
end

local A = {
    {4, 1, 1},
    {2, 3, 1},
    {1, 2, 3}
}
local b = {30, 60, 40}
local c = {3, 2, 1}

local opt_sol, opt_val = simplex(A, b, c)

for i = 1, #opt_sol do
    print("x" .. i .. " = " .. opt_sol[i])
end

print("Optimal objective function value: " .. opt_val)
