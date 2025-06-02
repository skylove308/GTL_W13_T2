
setmetatable(_ENV, { __index = EngineTypes })

local FVector = EngineTypes.FVector

AnimFSM = {
    Update = function(self, dt)
        state_idle = "Contents/JamesIdle/Armature|Idle"
        state_walk = "Contents/DavidSlowRun/Armature|SlowRun"
        state_run = "Contents/DavidFastRun/Armature|FastRun"
        state_die = "Contents/JamesDie/Armature|Die"

        speed = self.OwnerCharacter.Velocity;
        isRunning = self.OwnerCharacter.IsRunning;
        isDead = self.OwnerCharacter.IsDead;

        if isDead then
            current_state = state_die
        elseif speed <= 0.0 then
            current_state = state_idle
        else
            if isRunning then
                current_state = state_run
            else
                current_state = state_walk
            end
        end

        if current_state == state_die then
            state_by_blend = 1.2
        else
            state_by_blend = 0.5
        end

        return {
            anim = current_state,
            blend = state_by_blend
        }
    end
}

return AnimFSM
