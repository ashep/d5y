import Box from "@material-ui/core/Box";
import CircularProgress from "@material-ui/core/CircularProgress";
import messages from "./messages";

export default function APConnection(props) {
    return <Box className={'apConnInfo'} hidden={props.apConnected}>
        <Box className={"apConnHelp"}>
            <p dangerouslySetInnerHTML={{__html: messages.t('apConnHelp')}}/>
            <CircularProgress className={'connProgress'}/>
        </Box>
    </Box>
}
