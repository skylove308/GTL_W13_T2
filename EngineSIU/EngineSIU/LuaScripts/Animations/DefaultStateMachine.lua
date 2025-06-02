AnimFSM = {
    Update = function(self, dt)
        state_idle = "Contents/JamesIdle/Armature|Idle"
        state_walk = "Contents/DavidSlowRun/Armature|SlowRun"
        state_run = "Contents/DavidFastRun/Armature|FastRun"

        speed = self.OwnerCharacter.Speed;

        if speed == 6.0 then
            current_state = state_idle;
        end

        if current_state == state_idle then
            if speed > 6.0 then
                current_state = state_walk
            end
        elseif current_state == state_walk then
            if speed == 6.0 then
                current_state = state_idle
            end
            if speed > 8.0 then
                current_state = state_run
            end
        elseif current_state == state_run then
            if speed < 8.0 then
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