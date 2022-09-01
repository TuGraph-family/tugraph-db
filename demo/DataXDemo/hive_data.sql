CREATE TABLE movie
(
    id           int,
    duration     int,
    title        string,
    tagline      string,
    summary      string,
    poster_image string,
    rated        string
) ROW FORMAT DELIMITED FIELDS TERMINATED BY ','
STORED AS TEXTFILE;

INSERT INTO movie
    (id, duration, title, tagline, summary, poster_image, rated)
VALUES (100004, 14, "title4", "tagline4", "summary4", "poster_image4", "rated4"),
       (100005, 15, "title5", "tagline5", "summary5", "poster_image5", "rated5"),
       (100006, 16, "title6", "tagline6", "summary6", "poster_image6", "rated6");