import Box from "@material-ui/core/Box";
import CircularProgress from "@material-ui/core/CircularProgress";
import messages from "./messages";
import Typography from "@material-ui/core/Typography";

export default function APConnection(props) {
    return <Box className={'apConnInfo'} hidden={props.apConnected}>
        <Box className={"apConnHelp"}>
            <Typography>
                <p dangerouslySetInnerHTML={{__html: messages.t('apConnHelp')}}/>
            </Typography>

            <Typography align={"center"}>
                <CircularProgress className={'connProgress'}/>
            </Typography>
        </Box>
    </Box>
}
