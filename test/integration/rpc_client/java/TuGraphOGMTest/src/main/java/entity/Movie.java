package entity;

import java.util.HashSet;
import java.util.Set;
import com.antgroup.tugraph.ogm.annotation.Id;
import com.antgroup.tugraph.ogm.annotation.NodeEntity;
import com.antgroup.tugraph.ogm.annotation.Relationship;

@NodeEntity
public class Movie {

    @Id
    private Long id;
    private String title;
    private int released;

    @Relationship(type = "ACTS_IN", direction = Relationship.Direction.INCOMING)
    Set<Actor> actors = new HashSet<>();

    public Movie() {
    }

    public Movie(String title, int year) {
        this.title = title;
        this.released = year;
    }

    public Long getId() {
        return id;
    }


    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public int getReleased() {
        return released;
    }

    public void setReleased(int released) {
        this.released = released;
    }

    public Set<Actor> getActors() {
        return actors;
    }

    public void setActors(Set<Actor> actors) {
        this.actors = actors;
    }

}
