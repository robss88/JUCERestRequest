/*
  ==============================================================================

    RestRequest.h
    Created: 16 Dec 2015 2:29:23pm
    Author:  Adam Wilson

  ==============================================================================
*/

#ifndef RESTREQUEST_H_INCLUDED
#define RESTREQUEST_H_INCLUDED

class RestRequest
{
public:

    RestRequest (String urlString) : url (urlString) {}
    RestRequest (URL url)          : url (url) {}
    RestRequest () {}

    struct Response
    {
        Result result;
        StringPairArray headers;
        var body;
        String bodyAsString;
        int status;

        Response() : result (Result::ok()), status (0) {} // not sure about using Result if we have to initialise it to ok...
    } response;


    RestRequest::Response execute ()
    {
        auto urlRequest = url.getChildURL (endpoint);
        bool hasFields = (fields.getProperties().size() > 0);
        if (hasFields)
        {
            MemoryOutputStream output;
            JSON::FormatOptions formatOptions = JSON::FormatOptions().withIndentLevel(0).withMaxDecimalPlaces(20).withSpacing(JSON::Spacing::singleLine);

            fields.writeAsJSON (output, formatOptions);  // Updated to use the new API
            auto postBodyAsString = output.toString();
            DBG(postBodyAsString);
            urlRequest = urlRequest.withPOSTData (postBodyAsString);
        }

        std::unique_ptr<InputStream> input (urlRequest.createInputStream (hasFields, nullptr, nullptr, stringPairArrayToHeaderString(headers), 0, &response.headers, &response.status, 5, verb));

        response.result = checkInputStream (input);
        if (response.result.failed()) return response;

        response.bodyAsString = input->readEntireStreamAsString();
        response.result = JSON::parse(response.bodyAsString, response.body);

        return response;
    }
    
    RestRequest::Response execute (const var& bodyVar)
    {
        auto urlRequest = url.getChildURL (endpoint);
        
        String jsonBody = JSON::toString(bodyVar, false);
        urlRequest = urlRequest.withPOSTData(jsonBody);

        std::unique_ptr<InputStream> input (urlRequest.createInputStream (true, nullptr, nullptr, stringPairArrayToHeaderString(headers), 0, &response.headers, &response.status, 5, verb));

        response.result = checkInputStream (input);
        if (response.result.failed()) return response;

        response.bodyAsString = input->readEntireStreamAsString();
        response.result = JSON::parse(response.bodyAsString, response.body);

        return response;
    }

    RestRequest get (const String& endpoint = "")
    {
        RestRequest req (*this);
        req.verb = "GET";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest post (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "POST";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest put (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "PUT";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest del (const String& endpoint)
    {
        RestRequest req (*this);
        req.verb = "DELETE";
        req.endpoint = endpoint;

        return req;
    }

    RestRequest field (const String& name, const var& value)
    {
        fields.setProperty(name, value);
        return *this;
    }

    RestRequest header (const String& name, const String& value)
    {
        RestRequest req (*this);
        headers.set (name, value);
        return req;
    }
    
    void setURL(const String& urlString) {
        url = URL(urlString);
    }
    
    const URL& getURL() const
    {
        return url;
    }

    const String& getBodyAsString() const
    {
        return bodyAsString;
    }

    void clearFields() {
        fields.clear();
    }
private:
    URL url;
    StringPairArray headers;
    String verb;
    String endpoint;
    DynamicObject fields;
    String bodyAsString;

    Result checkInputStream (std::unique_ptr<InputStream>& input)
    {
        if (! input) return Result::fail ("HTTP request failed, check your internet connection");
        return Result::ok();
    }

    static String stringPairArrayToHeaderString(StringPairArray stringPairArray)
    {
        String result;
        for (auto key : stringPairArray.getAllKeys())
        {
            result += key + ": " + stringPairArray.getValue(key, "") + "\n";
        }
        return result;
    }
};



#endif  // RESTREQUEST_H_INCLUDED
