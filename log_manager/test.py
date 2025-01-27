import os
import sys
import time
import signal

pipe_r1, pipe_w1 = os.pipe() 
pipe_r2, pipe_w2 = os.pipe() 
pipe_r3, pipe_w3 = os.pipe() 
pipe_r4, pipe_w4 = os.pipe() 

current_directory = os.getcwd()

print("Current directory:", current_directory)


pid = os.fork()

if pid > 0:
    os.close(pipe_r1)
    os.close(pipe_r2)
    os.close(pipe_r3)
    os.close(pipe_r4)

    # pipe_w1 = os.fdopen(pipe_w1, 'w')
    # pipe_w2 = os.fdopen(pipe_w2, 'w')
    # pipe_w3 = os.fdopen(pipe_w3, 'w')
    # pipe_w4 = os.fdopen(pipe_w4, 'w')

    messages = [b"Hello from parent (message 1)!\n", b"This is another message.\n", b"End of messages.\n"]
    for msg in messages:
        os.write(pipe_w1, msg)
        os.write(pipe_w2, msg)
        os.write(pipe_w3, msg)
        os.write(pipe_w4, msg)
        time.sleep(3)

    # os.close(pipe_w1)
    # os.close(pipe_w2)
    # os.close(pipe_w3)
    # os.close(pipe_w4)

    time.sleep(60*7)
    for msg in messages:
        os.write(pipe_w1, msg)
        os.write(pipe_w2, msg)
        os.write(pipe_w3, msg)
        os.write(pipe_w4, msg)
        time.sleep(3)
        
    os.kill(pid, signal.SIGTERM)

elif pid == 0:
    # os.close(pipe_w1)
    # os.close(pipe_w2)
    # os.close(pipe_w3)
    # os.close(pipe_w4)

    os.dup2(pipe_r1, 0)
    os.dup2(pipe_r2, 1)
    os.dup2(pipe_r3, 2)
    os.dup2(pipe_r4, 3)

    for i in range(5,11):
        os.close(i)

    # Execute a new program (in this case, we use "cat" to read from stdin and output to stdout)
    exec_path = os.path.join(current_directory, "log_manager")
    os.execve(exec_path, ["lod_manager", "1", "test1", "test2"], os.environ)  # Executes 'cat', which reads from stdin and writes to stdout
    print("Failed to call")

else:
    print("Fork failed", file=sys.stderr)