import {createMuiTheme, ThemeProvider} from '@material-ui/core/styles';

import './App.css';
import Container from '@material-ui/core/Container';
import APConnection from "./APConnection";
import WiFiConnection from "./WiFiConnection";
import Connection from "./Connection";

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
                <Connection>
                    <APConnection/>
                    <WiFiConnection/>
                </Connection>
            </Container>
        </ThemeProvider>
    )
}

export default App;
