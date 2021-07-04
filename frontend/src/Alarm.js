import messages from "./messages";

import Box from "@material-ui/core/Box";
import TextField from "@material-ui/core/TextField";


export default function Alarm(props) {
    return <Box>
        <h2>{messages.t("titleAlarm")}</h2>

        <TextField
            id="time"
            label="Alarm clock"
            type="time"
            fullWidth={true}
            defaultValue="07:30"
            InputLabelProps={{
                shrink: true,
            }}
            inputProps={{
                step: 300, // 5 min
            }}
        />
    </Box>
}
