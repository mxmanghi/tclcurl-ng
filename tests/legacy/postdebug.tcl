::curl::transfer -post 1 -verbose 1 -url http://127.0.0.1/~manghi/index.rvt?form=submit&dup=2 \
        -httppost [list name "text_entry" contents "TCLCURL" contenttype "text/plain"] \
        -httppost [list name "option_selected" contents "opt1"] \
        -httppost [list name "submit" contents "Search"]
