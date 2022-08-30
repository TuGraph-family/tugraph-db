CREATE
DATABASE datax;

CREATE TABLE datax.movie
(
    id           INT          NOT NULL,
    duration     INT          NOT NULL,
    title        VARCHAR(100) NOT NULL,
    tagline      VARCHAR(100) NOT NULL,
    summary      VARCHAR(100) NOT NULL,
    poster_image VARCHAR(100) NOT NULL,
    rated        VARCHAR(100) NOT NULL,
    PRIMARY KEY (id)
);

INSERT INTO datax.movie
    (id, duration, title, tagline, summary, poster_image, rated)
VALUES (100001, 11, "title1", "tagline1", "summary1", "poster_image1", "rated1"),
       (100002, 12, "title2", "tagline2", "summary2", "poster_image2", "rated2"),
       (100003, 13, "title3", "tagline3", "summary3", "poster_image3", "rated3");
