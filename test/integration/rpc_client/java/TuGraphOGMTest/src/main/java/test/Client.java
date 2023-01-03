package test;

import com.antgroup.tugraph.ogm.config.Configuration;
import com.antgroup.tugraph.ogm.driver.Driver;
import com.antgroup.tugraph.ogm.drivers.rpc.driver.RpcDriver;

public class Client {
    private static Configuration.Builder baseConfigurationBuilder;

    protected static Driver getDriver(String[] args) {
        String databaseUri = "list://" + args[0];
        String username = args[1];
        String password = args[2];

        Driver driver = new RpcDriver();
        baseConfigurationBuilder = new Configuration.Builder()
            .uri(databaseUri)
            .verifyConnection(true)
            .credentials(username, password);
        driver.configure(baseConfigurationBuilder.build());
        return driver;
    }
}
