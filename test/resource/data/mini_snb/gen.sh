for i in comment.csv  forum_hasMember_person.csv  person_isLocatedIn_place.csv  post.csv  comment_hasCreator_person.csv  forum_hasModerator_person.csv  person_knows_person.csv  post_hasCreator_person.csv  comment_hasTag_tag.csv  forum_hasTag_tag.csv  person_likes_comment.csv  post_hasTag_tag.csv  comment_isLocatedIn_place.csv  import.conf  person_likes_post.csv  post_isLocatedIn_place.csv  comment_replyOf_comment.csv  organisation.csv  person_studyAt_organisation.csv  tagclass.csv  comment_replyOf_post.csv  organisation_isLocatedIn_place.csv  person_workAt_organisation.csv  tagclass_isSubclassOf_tagclass.csv  forum_containerOf_post.csv  person.csv  place.csv  tag.csv  forum.csv  person_hasInterest_tag.csv  place_isPartOf_place.csv  tag_hasType_tagclass.csv ; do
  head -n 100 ../../../../../snb/import_data/$i > $i
done

ln -sf comment.csv comment_hasCreator_person.csv
ln -sf comment.csv comment_isLocatedIn_place.csv
ln -sf post.csv forum_containerOf_post.csv
ln -sf forum.csv forum_hasModerator_person.csv
ln -sf organisation.csv organisation_isLocatedIn_place.csv
ln -sf person.csv person_isLocatedIn_place.csv
ln -sf post.csv post_hasCreator_person.csv
ln -sf post.csv post_isLocatedIn_place.csv
ln -sf tag.csv tag_hasType_tagclass.csv
