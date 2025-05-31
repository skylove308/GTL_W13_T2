AnimFSM = {
    current = "Idle",
    
    Update = function(self, dt)
        state_idle = "Contents/Human/Idle"
        state_walk = "Contents/Human/SlowRun"
        state_run = "Contents/Human/FastRun"

        self.current = "Contents/Human/Idle"
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