package test;

import org.neo4j.ogm.config.Configuration;
import org.neo4j.ogm.driver.Driver;
import org.neo4j.ogm.drivers.rpc.driver.RpcDriver;

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
