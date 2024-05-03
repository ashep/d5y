import * as React from "react";
import Stack from "@mui/material/Stack";
import Box from "@mui/material/Box";
import CircularProgress from '@mui/material/CircularProgress';

type WiFiListProps = {
    hidden: boolean;
    list: Array<string>;
    onItemClick: (id: string) => void;
}

type WiFiListState = {}

export default class WiFiList extends React.Component<WiFiListProps, WiFiListState> {
    constructor() {
        super();
    }

    render() {
        return (
            <Stack direction="column" spacing={1} sx={{ display: this.props.hidden ? "none" : "flex" }}>
                <Box sx={{textAlign: "center", display: this.props.list.length == 0 ? "block" : "none"}}>
                    <CircularProgress/>
                </Box>

                <Box>
                    <ul>
                        {this.props.list.map(item =>
                            <li>
                                <a href={"#"} onClick={_ => this.props.onItemClick(item)}>{item}</a>
                            </li>
                        )}
                    </ul>
                </Box>
            </Stack>
        )
    }
};