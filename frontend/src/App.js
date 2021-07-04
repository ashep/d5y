import {createMuiTheme, ThemeProvider} from '@material-ui/core/styles';

import './App.css';
import logo from './assets/logo.svg';


import Container from '@material-ui/core/Container';
import Box from '@material-ui/core/Box';

import APConnection from "./APConnection";
import WiFiConnection from "./WiFiConnection";


import Connection from "./Connection";
import Alarm from "./Alarm";
import Brightness from "./Brightness";

function App() {
    const theme = createMuiTheme({
        palette: {
            type: 'dark',
            primary: {
                main: '#a8dadc'
            }
        },
    });

    return (
        <ThemeProvider theme={theme}>
            <Container maxWidth={"sm"} className={"App"}>
                <Box className="Logo">
                    <img src={logo} alt="Logo"/>
                </Box>

                <Connection>
                    <APConnection/>
                    {/*<Alarm/>*/}
                    {/*<Brightness/>*/}
                    <WiFiConnection/>
                </Connection>

            </Container>
        </ThemeProvider>
    )
}

export default App;
