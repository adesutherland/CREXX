/* rexx */
options levelb comments_dash
import data_KeyDB
import rxfnsb

kdb = .KeyDB()

handle = kdb.open("kdb.db", "w+")
if handle < 0 then do
    say "Failed to openkey database"
    exit 1
end

/* Begin transaction */
rc = kdb.begin()

/* kdb.put some test data */
rc =  kdb.put("Tokyo", "Japan")
rc =  kdb.put("New York City", "United States")
rc =  kdb.put("London", "United Kingdom")
rc =  kdb.put("Paris", "France")
rc =  kdb.put("Berlin", "Germany")
rc =  kdb.put("Madrid", "Spain")
rc =  kdb.put("Rome", "Italy")
rc =  kdb.put("Beijing", "China")
rc =  kdb.put("Moscow", "Russia")
rc =  kdb.put("Sydney", "Australia")
rc =  kdb.put("Toronto", "Canada")
rc =  kdb.put("São Paulo", "Brazil")
rc =  kdb.put("Buenos Aires", "Argentina")
rc =  kdb.put("Cairo", "Egypt")
rc =  kdb.put("Cape Town", "South Africa")
rc =  kdb.put("Istanbul", "Turkey")
rc =  kdb.put("Mumbai", "India")
rc =  kdb.put("Bangkok", "Thailand")
rc =  kdb.put("Jakarta", "Indonesia")
rc =  kdb.put("Seoul", "South Korea")
rc =  kdb.put("Dubai", "United Arab Emirates")
rc =  kdb.put("Singapore", "Singapore")
rc =  kdb.put("Athens", "Greece")
rc =  kdb.put("Lisbon", "Portugal")
rc =  kdb.put("Dublin", "Ireland")
rc =  kdb.put("Amsterdam", "Netherlands")
rc =  kdb.put("Warsaw", "Poland")
rc =  kdb.put("Vienna", "Austria")
rc =  kdb.put("Zurich", "Switzerland")
rc =  kdb.put("Oslo", "Norway")
rc =  kdb.put("Stockholm", "Sweden")
rc =  kdb.put("Helsinki", "Finland")
rc =  kdb.put("Reykjavik", "Iceland")
rc =  kdb.put("Nairobi", "Kenya")
rc =  kdb.put("Casablanca", "Morocco")
rc =  kdb.put("Tehran", "Iran")
rc =  kdb.put("Baghdad", "Iraq")
rc =  kdb.put("Riyadh", "Saudi Arabia")
rc =  kdb.put("Caracas", "Venezuela")
rc =  kdb.put("Lima", "Peru")
rc =  kdb.put("Santiago", "Chile")
rc =  kdb.put("Bogota", "Colombia")
rc =  kdb.put("Havana", "Cuba")
rc =  kdb.put("Mexico City", "Mexico")
rc =  kdb.put("Kuala Lumpur", "Malaysia")
rc =  kdb.put("Ho Chi Minh City", "Vietnam")
rc =  kdb.put("Manila", "Philippines")
rc =  kdb.put("Auckland", "New Zealand")
rc =  kdb.put("Hong Kong", "China")
rc =  kdb.put("Los Angeles", "United States")

/* Commit transaction */
rc =  kdb.commit()

/* get the size */
say
count = kdb.size()
say count "keys found"

/* readkey and display values */
say ""
say "readkey values:"
say "City:" kdb.get("Lima")
say "City:" kdb.get("Bogota")

/* Delete a key */
say ""
say "Deleting Berlin..."
rc = kdb.begin()
rc = kdb.remove("Berlin")
rc = kdb.commit()

/* listkey keys again */
say ""
say "Keys after deletion:"
count = kdb.size()
say count "keys found"

/* Show statistics */
say ""
say "Database statistics:"
say kdb.stats()

