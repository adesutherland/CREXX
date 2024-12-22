/* XML Processing Examples */
options levelb
import rxml
import rxfnsb


/* Initialize test suite */
call section 'XML Processing Test Suite', ,
            'A comprehensive test suite for XML processing functionality'
total_tests = 0
passed_tests = 0

/* Example 1 */
call section 'EXAMPLE 1: Parse and Find Elements', ,
            'Tests basic XML parsing and element finding capabilities'

xml = ,
'<?xml version="1.0"?>' || ,
'<bookstore>' || ,
'  <book>' || ,
'    <title>The Great Gatsby</title>' || ,
'    <author>F. Scott Fitzgerald</author>' || ,
'    <price>10.99</price>' || ,
'  </book>' || ,
'  <book>' || ,
'    <title>1984</title>' || ,
'    <author>George Orwell</author>' || ,
'    <price>9.99</price>' || ,
'  </book>' || ,
'</bookstore>'

total_tests = total_tests + 1

/* Parse the XML */
if xmlparse(xml) < 0 then do
    call summarize 'XML Parsing', 'FAILED', xmlerror()
    exit 1
end
else do
    passed_tests = passed_tests + 1
    call summarize 'XML Parsing', 'PASSED',''
end

call subsection 'Finding Titles'
titles[1] = ''
count = xmlfind('title', titles)
total_tests = total_tests + 1
if count >0 then do
    passed_tests = passed_tests + 1
    say 'Found' count 'titles:'
    do i = 1 to count
        say '  ►' titles[i]
    end
end
else call summarize 'Title Finding', 'FAILED', 'Expected 2 titles, found' count

call subsection 'Finding Authors'
authors[1] = ''
count = xmlfind('author', authors)
total_tests = total_tests + 1
if count = 2 then do
    passed_tests = passed_tests + 1
    say 'Found' count 'authors:'
    do i = 1 to count
        say '  ►' authors[i]
    end
end
else
    call summarize 'Author Finding', 'FAILED', 'Expected 2 authors, found' count

/* Example 2 */
call section 'EXAMPLE 2: Build XML', ,
            'Tests XML building functionality with simple elements'

elements[1] = 'name'
elements[2] = 'John Doe'
elements[3] = 'age'
elements[4] = '30'
elements[5] = 'email'
elements[6] = 'john@example.com'

newxml = xmlbuild('person', elements)
call subsection 'Generated Simple XML'
say newxml

total_tests = total_tests + 1
if xmlparse(newxml) = 0 then do
    passed_tests = passed_tests + 1
    call summarize 'XML Building', 'PASSED',''
end
else
    call summarize 'XML Building', 'FAILED', xmlerror()

/* Example 3 */
call section 'EXAMPLE 3: Error Handling', ,
            'Tests error detection and handling capabilities'

badxml = '<unclosed>test'
total_tests = total_tests + 1
if xmlparse(badxml) < 0 then do
    passed_tests = passed_tests + 1
    call summarize 'Error Handling', 'PASSED', 'Correctly detected:' xmlerror()
end
else
    call summarize 'Error Handling', 'FAILED', 'Failed to detect unclosed tag'

/* Example 4 */
call section 'EXAMPLE 4: Complex XML Building', ,
            'Tests nested XML structures and complex data handling'

call subsection 'Building Contact Records'
contact1[1] = 'name'
contact1[2] = 'Alice Smith'
contact1[3] = 'phone'
contact1[4] = '123-456-7890'
contact1[5] = 'email'
contact1[6] = 'alice@example.com'

contact2[1] = 'name'
contact2[2] = 'Bob Jones'
contact2[3] = 'phone'
contact2[4] = '987-654-3210'
contact2[5] = 'email'
contact2[6] = 'bob@example.com'

alice = xmlbuild('contact', contact1)
bob = xmlbuild('contact', contact2)

contacts[1] = 'contact'
contacts[2] = alice
contacts[3] = 'contact'
contacts[4] = bob

list = xmlbuild('contacts', contacts)

call subsection 'Generated Contact List'
say list

total_tests = total_tests + 1
if xmlparse(list) = 0 then do
    passed_tests = passed_tests + 1
    names[1] = ''
    count = xmlfind('name', names)

    call subsection 'Verified Contacts'
    say 'Found' count 'contacts:'
    do i = 1 to count
        say '  ►' names[i]
    end

    call subsection 'Phone Numbers'
    phones[1] = ''
    count = xmlfind('phone', phones)
    say 'Found' count 'phone numbers:'
    do i = 1 to count
        say '  ►' phones[i]
    end

    call summarize 'Complex XML', 'PASSED',''
end
else do
    call summarize 'Complex XML', 'FAILED', xmlerror()
end

/* Test Suite Summary */
call section 'Test Suite Summary', ,
            'Overall results of the XML processing test suite'
say 'Total tests run:' total_tests
say 'Tests passed:  ' passed_tests

/* Test attribute handling */
say "Testing XML Attributes"

say "Test 1: Simple product with attributes"
xml = '<products>' || ,
      '<product sku="ABC123" price="29.99" in-stock="true"/>' || ,
      '<product sku="XYZ789" price="15.50" in-stock="false"/>' || ,
      '</products>'

if xmlparse(xml) = 0 then say "parsing successful"
else say "Error parsing XML:" xmlerror()

/* Find products */
elements1 = ""
count = xmlfind("product", elements)
if count = 2 then do
   sku = ""
   price = ""
   stock = ""
   say "Attribute count "count
   /* Test first product attributes */
   sku = xmlgetattr(1, "sku")
   price = xmlgetattr(1, "price")
   stock = xmlgetattr(1, "in-stock")
   say "Product 1: SKU=" sku "Price=" price "In Stock=" stock

   /* Test second product attributes */
   sku = xmlgetattr(2, "sku")
   price = xmlgetattr(2, "price")
   stock = xmlgetattr(2, "in-stock")
   say "Product 2: SKU=" sku "Price=" price "In Stock=" stock

   /* Test attribute modification */
   if xmlsetattr(1, "price", "39.99") = 0 then
   say "Updated price:" xmlgetattr(1, "price")
   /* Test second product attributes */
   sku = xmlgetattr(1, "sku")
   price = xmlgetattr(1, "price")
   stock = xmlgetattr(1, "in-stock")
   say "Product 1: SKU=" sku "Price=" price "In Stock=" stock

end


say "Test 2: Complex nested elements with attributes"
xml = '<user id="12345" role="admin">' || ,
      '<preferences theme="dark" notifications="enabled">' || ,
      '<setting name="email-alerts" value="true"></setting>' || ,
      '</preferences>' || ,
      '</user>'

if xmlparse(xml) \= 0 then
   say "Error parsing XML:" xmlerror()

elements1 = ""
count = xmlfind("user", elements)
if count = 1 then do
   userid = ""
   role = ""
   theme = ""
   notif = ""
   attrcount = 0

   /* Test user attributes */
   userid = xmlgetattr(1, "id")
   role = xmlgetattr(1, "role")
   say "User: ID=" userid "Role=" role

   /* Find and test preferences */
   elements1 = ""
   prefcount = xmlfind("preferences", elements)
   if prefcount = 1 then do
      theme = xmlgetattr(1, "theme")
      notif = xmlgetattr(1, "notifications")
      say "Preferences: Theme=" theme "Notifications=" notif

      /* Test attribute removal */
      if xmlremattr(1, "notifications") = 0 then do
         attrcount = xmlattrcount(1)
         say "Attributes after removal:" attrcount
      end
   end
end

say "Test 3: Special characters in attributes"
xml = '<entry date="2024-03-15" author="O''Neil" note="Quote: ""Hello"""></entry>'

if xmlparse(xml) \= 0 then
   say "Error parsing XML:" xmlerror()

elements1 = ""
count = xmlfind("entry", elements)
if count = 1 then do
   date = ""
   author = ""
   note = ""

   date = xmlgetattr(1, "date")
   author = xmlgetattr(1, "author")
   note = xmlgetattr(1, "note")
   say "Entry: Date=" date "Author=" author "Note=" note
end

say "Attribute Tests Complete"
exit 0

/* Helper function for section headers */
section: procedure
    arg title = .string, description = .string
    say
    say copies('=',72)
    say center(' 'title' ', 72)
    if description \= '' then do
        say copies('-',72)
        say description
    end
    say copies('=',72)
    say
    return

/* Helper function for subsections */
subsection: procedure
    arg title = .string
    say
    say copies('-',72)
    say center(' 'title' ', 72)
    say copies('-',72)
    return

/* Helper function for result summary */
summarize: procedure
    arg test_name = .string, status = .string, details = .string
    say
    say '► Result for' test_name':'
    say '  Status:' status
    if details \= '' then
        say '  Details:' details
    return

/* XML Cleaning and Processing */
/*
cleanxml: procedure = .string
    arg xmltext = .string
    outxml = ''
    inside_tag = 0
    last_char = ''
    tag_name = ''
    collecting_tag = 0

   /* Remove XML declaration if present */
        if pos('<?xml', xmltext) = 1 then do
            p = pos('?>', xmltext)
            if p > 0 then xmltext = substr(xmltext, p+2)
        end

    do i = 1 to length(xmltext)
        char = substr(xmltext, i, 1)

        if char = '<' then do
           inside_tag = 1
           collecting_tag = 1
           tag_name = ''
           outxml = outxml || char
        end
        else if char = '/' & inside_tag then do
           if substr(xmltext, i+1, 1) = '>' then do
              /* Found '/>', replace with '></tag>' */
              outxml = outxml || '></' || strip(tag_name) || '>'
              i = i + 1  /* Skip the next '>' */
              inside_tag = 0
              collecting_tag = 0
           end
           else outxml = outxml || char
        end
        else if char = '>' then do
           inside_tag = 0
           collecting_tag = 0
           outxml = outxml || char
        end
        else if verify(char, ' ' || '0a0d09'x) = 0 then do
           if \inside_tag & last_char \= '>' then outxml = outxml || ' '
           if inside_tag then outxml = outxml || char
           collecting_tag = 0
        end
        else do
           outxml = outxml || char
           if collecting_tag then do
              if verify(char, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-') = 0 then
                 tag_name = tag_name || char
              else if char = ' ' then
                 collecting_tag = 0
           end
        end

        if verify(char, ' ' || '0a0d09'x) \= 0 then last_char = char
    end

    outxml = space(outxml)
    say "cleansed "outxml
    return outxml
    */