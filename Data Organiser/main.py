import Organiser
import Visualizer


def main_menu():

    while True:
        print(
            f"""\033[1;34m
 __  __        _         __  __                 
|  \/  | __ _ (_) _ _   |  \/  | ___  _ _  _  _ 
| |\/| |/ _` || || ' \  | |\/| |/ -_)| ' \| || |
|_|  |_|\__,_||_||_||_| |_|  |_|\___||_||_|\_,_|
        \033[0m"""
        )
        print("Please choose an option\n")
        print(f"\033[1;34m[1]\033[0m Reset database")
        print(f"\033[1;34m[2]\033[0m Organise collected data")
        print(f"\033[1;34m[3]\033[0m Generate general graphs")
        print(f"\033[1;34m[4]\033[0m Generate player specific graphs")
        print(f"\033[1;34m[Anything]\033[0m Exit")
        print()

        choice = input("Enter your choice:\033[1;34m ")
        print(f"\033[0m")

        if choice == "1":
            Organiser.unorganise_main()
        elif choice == "2":
            Organiser.organise_main()
        elif choice == "3":
            Visualizer.general_graphs()
        elif choice == "4":
            Visualizer.player_specific_graphs()
        else:
            print("Exiting...")
            break


if __name__ == "__main__":
    main_menu()
