/* REXX Llistkey plugin */
options levelb
import keyaccess
import rxfnsb
/* Test script for fileio plugin */

/* openkey database */
say "openkeying database..."
handle = openkey("c:/temp/crexx/test.db", "w+")
if handle < 0 then do
    say "Failed to openkey database"
    exit 1
end

/* Begin transaction */
call txbegin handle

/* writekey some test data */
call writekey handle,"Tokyo", "Japan"
call writekey handle,"New York City", "United States"
call writekey handle,"London", "United Kingdom"
call writekey handle,"Paris", "France"
call writekey handle,"Berlin", "Germany"
call writekey handle,"Madrid", "Spain"
call writekey handle,"Rome", "Italy"
call writekey handle,"Beijing", "China"
call writekey handle,"Moscow", "Russia"
call writekey handle,"Sydney", "Australia"
call writekey handle,"Toronto", "Canada"
call writekey handle,"São Paulo", "Brazil"
call writekey handle,"Buenos Aires", "Argentina"
call writekey handle,"Cairo", "Egypt"
call writekey handle,"Cape Town", "South Africa"
call writekey handle,"Istanbul", "Turkey"
call writekey handle,"Mumbai", "India"
call writekey handle,"Bangkok", "Thailand"
call writekey handle,"Jakarta", "Indonesia"
call writekey handle,"Seoul", "South Korea"
call writekey handle,"Dubai", "United Arab Emirates"
call writekey handle,"Singapore", "Singapore"
call writekey handle,"Athens", "Greece"
call writekey handle,"Lisbon", "Portugal"
call writekey handle,"Dublin", "Ireland"
call writekey handle,"Amsterdam", "Netherlands"
call writekey handle,"Warsaw", "Poland"
call writekey handle,"Vienna", "Austria"
call writekey handle,"Zurich", "Switzerland"
call writekey handle,"Oslo", "Norway"
call writekey handle,"Stockholm", "Sweden"
call writekey handle,"Helsinki", "Finland"
call writekey handle,"Reykjavik", "Iceland"
call writekey handle,"Nairobi", "Kenya"
call writekey handle,"Casablanca", "Morocco"
call writekey handle,"Tehran", "Iran"
call writekey handle,"Baghdad", "Iraq"
call writekey handle,"Riyadh", "Saudi Arabia"
call writekey handle,"Caracas", "Venezuela"
call writekey handle,"Lima", "Peru"
call writekey handle,"Santiago", "Chile"
call writekey handle,"Bogota", "Colombia"
call writekey handle,"Havana", "Cuba"
call writekey handle,"Mexico City", "Mexico"
call writekey handle,"Kuala Lumpur", "Malaysia"
call writekey handle,"Ho Chi Minh City", "Vietnam"
call writekey handle,"Manila", "Philippines"
call writekey handle,"Auckland", "New Zealand"
call writekey handle,"Hong Kong", "China"
call writekey handle,"Los Angeles", "United States"

/* Commit transaction */
call txcommit handle

/* listkey all keys */
say ""
say "listkeying all keys:"
count = listkey(handle)
say count "keys found"

/* readkey and display values */
say ""
say "readkey values:"
say "City:" readkey(handle, "Lima")
say "City:" readkey(handle, "Bogota")

/* Delete a key */
say ""
say "Deleting Berlin..."
call txbegin handle
call Deletekey handle, "Berlin"
call txcommit handle

/* listkey keys again */
say ""
say "Keys after deletion:"
count = listkey(handle)
say count "keys found"

/* Show statistics */
say ""
say "Database statistics:"
say stats(handle)

/* Validate database */
say ""
say "Validating database..."
errors = validate(handle)
if errors > 0 then
    say "Found" errors "errors"
else
    say "No errors found"

/* Create backup */
say ""
say "Creating backup..."
call backup handle, "test.db.backup"

/* Compact database */
say ""
say "Compacting database..."
call compact handle

/* closekey database */
say ""
say "Closing database..."
call closekey handle

exit 0