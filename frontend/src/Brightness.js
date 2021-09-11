import messages from "./messages";

import Box from "@material-ui/core/Box";
import Typography from "@material-ui/core/Typography";
import Slider from "@material-ui/core/Slider"

export default function Brightness(props) {
    return <Box>
        <h2>{messages.t("titleBrightness")}</h2>

        <Typography id="brightness-slider" gutterBottom>
            {messages.t("maxBrightness")}
        </Typography>

        <Slider
            defaultValue={16}
            // getAriaValueText={messages.t("titleBrightness")}
            aria-labelledby="brightness-slider"
            valueLabelDisplay="auto"
            step={1}
            marks
            min={1}
            max={16}
        />
    </Box>
}
