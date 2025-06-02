
setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

AnimFSM = {
    current = "Idle",
    
    Update = function(self, dt)
        state_idle = "Contents/DavidIdle/Armature|Idle"
        state_walk = "Contents/DavidSlowRun/Armature|SlowRun"
        state_run = "Contents/DavidFastRun/Armature|FastRun"

        self.current = "Contents/David/Armature|Idle"
        speed = self.OwnerCharacter.Velocity;
        isRunning = self.OwnerCharacter.IsRunning;

        if speed <= 0.0 then
            current_state = state_idle
        else
            if isRunning then
                current_state = state_run
            else
                current_state = state_walk
            end
        end

        return {
            anim = current_state,
            blend = 0.5
        }
    end
}

return AnimFSM