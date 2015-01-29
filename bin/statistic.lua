print("  sourcing statistic.lua")
-- statistic.lua
--
-- Example custom statistic


local StatisticTemplate = {}


function StatisticTemplate:finalize ()
end


function StatisticTemplate:apply (stat_dt)
end


function StatisticTemplate:display ()
end


function StatisticTemplate:reset ()
end


function StatisticTemplate:save ()
end


return StatisticTemplate    -- Return your custom statistic here



