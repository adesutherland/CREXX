/* REXX Llistkey plugin */
options levelb
import keyaccess
import rxfnsb
/* Test script for fileio plugin */

/* openkey database */
say "openkeying database..."
handle = _openkey("test.db", "w+") /* Just put it in the current directory (under the cmakefile, the current bin directory) */
if handle < 0 then do
    say "Failed to openkey database"
    exit 1
end

/* Begin transaction */
call _txbegin handle

/* _writekey some test data */
call _writekey handle,"Tokyo", "Japan"
call _writekey handle,"New York City", "United States"
call _writekey handle,"London", "United Kingdom"
call _writekey handle,"Paris", "France"
call _writekey handle,"Berlin", "Germany"
call _writekey handle,"Madrid", "Spain"
call _writekey handle,"Rome", "Italy"
call _writekey handle,"Beijing", "China"
call _writekey handle,"Moscow", "Russia"
call _writekey handle,"Sydney", "Australia"
call _writekey handle,"Toronto", "Canada"
call _writekey handle,"São Paulo", "Brazil"
call _writekey handle,"Buenos Aires", "Argentina"
call _writekey handle,"Cairo", "Egypt"
call _writekey handle,"Cape Town", "South Africa"
call _writekey handle,"Istanbul", "Turkey"
call _writekey handle,"Mumbai", "India"
call _writekey handle,"Bangkok", "Thailand"
call _writekey handle,"Jakarta", "Indonesia"
call _writekey handle,"Seoul", "South Korea"
call _writekey handle,"Dubai", "United Arab Emirates"
call _writekey handle,"Singapore", "Singapore"
call _writekey handle,"Athens", "Greece"
call _writekey handle,"Lisbon", "Portugal"
call _writekey handle,"Dublin", "Ireland"
call _writekey handle,"Amsterdam", "Netherlands"
call _writekey handle,"Warsaw", "Poland"
call _writekey handle,"Vienna", "Austria"
call _writekey handle,"Zurich", "Switzerland"
call _writekey handle,"Oslo", "Norway"
call _writekey handle,"Stockholm", "Sweden"
call _writekey handle,"Helsinki", "Finland"
call _writekey handle,"Reykjavik", "Iceland"
call _writekey handle,"Nairobi", "Kenya"
call _writekey handle,"Casablanca", "Morocco"
call _writekey handle,"Tehran", "Iran"
call _writekey handle,"Baghdad", "Iraq"
call _writekey handle,"Riyadh", "Saudi Arabia"
call _writekey handle,"Caracas", "Venezuela"
call _writekey handle,"Lima", "Peru"
call _writekey handle,"Santiago", "Chile"
call _writekey handle,"Bogota", "Colombia"
call _writekey handle,"Havana", "Cuba"
call _writekey handle,"Mexico City", "Mexico"
call _writekey handle,"Kuala Lumpur", "Malaysia"
call _writekey handle,"Ho Chi Minh City", "Vietnam"
call _writekey handle,"Manila", "Philippines"
call _writekey handle,"Auckland", "New Zealand"
call _writekey handle,"Hong Kong", "China"
call _writekey handle,"Los Angeles", "United States"

/* Commit transaction */
call _txcommit handle

/* listkey all keys */
say ""
say "listkeying all keys:"
count = _listkey(handle)
say count "keys found"

/* readkey and display values */
say ""
say "readkey values:"
say "City:" _readkey(handle, "Lima")
say "City:" _readkey(handle, "Bogota")

/* Delete a key */
say ""
say "Deleting Berlin..."
call _txbegin handle
call _deletekey handle, "Berlin"
call _txcommit handle

/* listkey keys again */
say ""
say "Keys after deletion:"
count = _listkey(handle)
say count "keys found"

/* Show statistics */
say ""
say "Database statistics:"
say _stats(handle)

/* Validate database */
say ""
say "Validating database..."
errors = _validate(handle)
if errors > 0 then
    say "Found" errors "errors"
else
    say "No errors found"

/* Create backup */
say ""
say "Creating backup..."
call _backup handle, "test.db.backup"

/* Compact database */
say ""
say "Compacting database..."
call _compact handle

/* closekey database */
say ""
say "Closing database..."
call _closekey handle

exit 0
